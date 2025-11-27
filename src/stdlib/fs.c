/* Aria_lang/src/stdlib/fs.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);
extern void* list_new();
extern void list_push(void* list, void* item);

// --- Tagging Helpers ---
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }

// Returns Tagged Integer (FD)
void* fs_open(void* path_tagged, void* mode_tagged) {
    char* path = unbox_str((Value)path_tagged);
    char* mode = unbox_str((Value)mode_tagged);
    if (!path || !mode) return (void*)box_int(-1);
    
    int flags = 0;
    if (strcmp(mode, "r") == 0) flags = O_RDONLY;
    else if (strcmp(mode, "w") == 0) flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (strcmp(mode, "a") == 0) flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (strcmp(mode, "rw") == 0) flags = O_RDWR | O_CREAT;
    else return (void*)box_int(-1);

    int fd = open(path, flags, 0644);
    return (void*)box_int(fd);
}

void fs_close(void* fd_tagged) {
    int fd = unbox_int((Value)fd_tagged);
    if (fd >= 0) close(fd);
}

// Returns Tagged String
void* fs_read_all(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)aria_alloc(length + 1);
    if (!buffer) { fclose(f); return NULL; }
    
    size_t read_len = fread(buffer, 1, length, f);
    buffer[read_len] = '\0';
    fclose(f);
    return (void*)box_ptr(buffer, TAG_STRING);
}

// Returns Tagged Integer (Success 1/0)
void* fs_write_str(void* path_tagged, void* content_tagged) {
    char* path = unbox_str((Value)path_tagged);
    char* content = unbox_str((Value)content_tagged);
    if (!path || !content) return (void*)box_int(0);
    
    FILE* f = fopen(path, "w");
    if (!f) return (void*)box_int(0);
    int res = fprintf(f, "%s", content);
    fclose(f);
    return (void*)box_int(res > 0);
}

void* fs_exists(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if(!path) return (void*)box_int(0);
    return (void*)box_int(access(path, F_OK) != -1);
}

void* fs_delete(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if(!path) return (void*)box_int(0);
    return (void*)box_int(unlink(path) == 0);
}

// Returns Tagged List
void* fs_list_dir(void* path_tagged) {
    char* path = unbox_str((Value)path_tagged);
    if (!path) return NULL;

    DIR* d = opendir(path);
    if (!d) return NULL;
    
    void* list = list_new();
    struct dirent* dir;
    
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
        
        size_t len = strlen(dir->d_name);
        char* name_copy = (char*)aria_alloc(len + 1);
        memcpy(name_copy, dir->d_name, len + 1);
        
        // Box string and push
        list_push(list, (void*)box_ptr(name_copy, TAG_STRING));
    }
    closedir(d);
    return list;
}
