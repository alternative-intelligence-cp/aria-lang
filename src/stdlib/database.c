/* Aria_lang/src/stdlib/database.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Runtime Imports: We utilize the existing runtime's object system for our Keydir index.
extern void* aria_alloc(size_t s);
extern void* aria_alloc_object();
extern void* aria_obj_set(void* o, char* k, void* v);
extern void* aria_obj_get(void* o, char* k);

// --- Tagging System Constants ---
// Aria uses NaN-boxing. We must conform to this ABI.
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define TAG_OBJECT      (TAG_BASE | 6ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

// Helper macros for boxing/unboxing
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }
static inline void* unbox_ptr(Value v) { return (void*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// --- Database Structures ---

// The Header stored immediately before every Key-Value pair on disk.
// Packed to ensure consistent binary layout across systems.
#pragma pack(push, 1)
typedef struct {
    uint32_t timestamp;
    uint32_t key_sz;
    uint32_t val_sz;
} LogHeader;
#pragma pack(pop)

// In-Memory Database Context
typedef struct {
    FILE* file_handle;
    char* filepath;
    // The Index: Maps Key (String) -> File Offset (Integer)
    // We leverage the existing highly-optimized AriaObject for this hash map.
    void* index_obj; 
} Database;

// Basic sanity check for record headers to prevent reading garbage
static int check_integrity(LogHeader* h) {
    // Arbitrary sanity limits: Keys < 1MB, Values < 64MB
    return (h->key_sz < 1024*1024) && (h->val_sz < 1024*1024*64); 
}

// --- Public API Implementation ---

// db_open(filepath: String) -> Database Object
// Opens a database file, creating it if necessary, and rebuilds the index.
void* db_open(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if (!path) return NULL;

    Database* db = (Database*)aria_alloc(sizeof(Database));
    db->filepath = (char*)aria_alloc(strlen(path) + 1);
    strcpy(db->filepath, path);
    
    // Allocate the index (Keydir)
    db->index_obj = aria_alloc_object();

    // Open for Append + Update (Binary)
    // "a+b": Appends writes to the end, but allows seeking for reads.
    db->file_handle = fopen(path, "a+b");
    if (!db->file_handle) return NULL;

    // --- Index Recovery Phase ---
    // Scan the entire file from the beginning to populate the in-memory index.
    fseek(db->file_handle, 0, SEEK_SET);
    
    long current_offset = 0;
    LogHeader header;
    
    // Read headers sequentially
    while (fread(&header, sizeof(LogHeader), 1, db->file_handle) == 1) {
        if (!check_integrity(&header)) {
            fprintf(stderr, " Warning: Skipping corrupt record at offset %ld\n", current_offset);
            // In a robust system, we might scan ahead for a valid sentinel, 
            // but here we stop recovery to prevent loading bad data.
            break; 
        }

        // Read the Key
        char* key_buf = (char*)aria_alloc(header.key_sz + 1);
        if (fread(key_buf, 1, header.key_sz, db->file_handle)!= header.key_sz) {
             // Unexpected EOF
             break;
        }
        key_buf[header.key_sz] = '\0';

        // Skip the Value (Performance optimization: don't read values into RAM during startup)
        fseek(db->file_handle, header.val_sz, SEEK_CUR);

        // Update Index: Key -> Offset of the RECORD START
        // We calculate the offset of the record start to allow db_get to re-read the header if needed.
        aria_obj_set(db->index_obj, key_buf, (void*)box_int((int32_t)current_offset));

        // Advance current_offset counter
        current_offset += sizeof(LogHeader) + header.key_sz + header.val_sz;
    }
    
    // Reset file pointer to the end for the next write operation
    fseek(db->file_handle, 0, SEEK_END);
    
    return (void*)box_ptr(db, TAG_OBJECT);
}

// db_put(db: Database, key: String, value: String)
// Appends a new Key-Value pair to the log and updates the index.
void db_put(void* db_tagged, void* k_tagged, void* v_tagged) {
    Database* db = (Database*)unbox_ptr((Value)db_tagged);
    char* key = unbox_str((Value)k_tagged);
    char* val = unbox_str((Value)v_tagged);

    // Validation
    if (!db ||!db->file_handle ||!key ||!val) return;

    // 1. Prepare the Header
    LogHeader header;
    header.timestamp = (uint32_t)time(NULL);
    header.key_sz = (uint32_t)strlen(key);
    header.val_sz = (uint32_t)strlen(val);

    // 2. Determine the Write Offset (This is where the data will live)
    // Since we opened in 'a' mode, writing always happens at EOF, 
    // but ftell gives us the linear address.
    fseek(db->file_handle, 0, SEEK_END);
    long write_offset = ftell(db->file_handle);

    // 3. Write Data Atomically (buffered)
    fwrite(&header, sizeof(LogHeader), 1, db->file_handle);
    fwrite(key, 1, header.key_sz, db->file_handle);
    fwrite(val, 1, header.val_sz, db->file_handle);
    
    // 4. Flush to Disk (Durability)
    // While fsync() is safer, fflush() is sufficient for this level of implementation
    // to ensure data leaves the C runtime buffers.
    fflush(db->file_handle);

    // 5. Update In-Memory Index
    // We copy the key to ensure it persists in the index object's lifecycle
    char* key_copy = (char*)aria_alloc(strlen(key) + 1);
    strcpy(key_copy, key);
    // Store the offset in the index
    aria_obj_set(db->index_obj, key_copy, (void*)box_int((int32_t)write_offset));
}

// db_get(db: Database, key: String) -> String | Null
// Retrieves a value by looking up the offset in the index and reading from disk.
void* db_get(void* db_tagged, void* k_tagged) {
    Database* db = (Database*)unbox_ptr((Value)db_tagged);
    char* key = unbox_str((Value)k_tagged);
    
    if (!db ||!key) return (void*)0; // Return Tagged Null

    // 1. Look up Offset in Index
    void* offset_boxed = aria_obj_get(db->index_obj, key);
    
    // Check validity of the result (0/NULL indicates key not found)
    Value off_val = (Value)offset_boxed;
    // Aria uses tagged integers. If the tag isn't Integer, it's not a valid offset.
    if ((off_val & TAG_BASE)!= TAG_INTEGER) return (void*)0;

    long offset = (long)unbox_int(off_val);

    // 2. Perform Random Read
    // We must save the current write position to restore it later
    long write_pos = ftell(db->file_handle);
    
    // Seek to the record
    if (fseek(db->file_handle, offset, SEEK_SET)!= 0) {
        return (void*)0; // Seek failed
    }
    
    // Read Header
    LogHeader header;
    if (fread(&header, sizeof(LogHeader), 1, db->file_handle)!= 1) {
        fseek(db->file_handle, write_pos, SEEK_SET); // Restore state
        return (void*)0;
    }

    // Skip the Key (we already know it)
    fseek(db->file_handle, header.key_sz, SEEK_CUR);

    // Read the Value
    char* val_buf = (char*)aria_alloc(header.val_sz + 1);
    if (fread(val_buf, 1, header.val_sz, db->file_handle)!= header.val_sz) {
        fseek(db->file_handle, write_pos, SEEK_SET); // Restore state
        return (void*)0; // Read failed
    }
    val_buf[header.val_sz] = '\0';

    // Restore the file pointer to the end for subsequent writes
    fseek(db->file_handle, write_pos, SEEK_SET);

    return (void*)box_ptr(val_buf, TAG_STRING);
}

// db_close(db: Database)
// Closes the file handle and releases resources.
void db_close(void* db_tagged) {
    Database* db = (Database*)unbox_ptr((Value)db_tagged);
    if (db && db->file_handle) {
        fclose(db->file_handle);
        db->file_handle = NULL;
    }
}
