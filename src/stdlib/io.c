/* Aria_lang/src/stdlib/io.c */
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

extern void* aria_alloc(size_t size);

// --- Tagging Helpers ---
typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline double unbox_double(Value v) { union { uint64_t u; double d; } u; u.u = v; return u.d; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_ptr(Value v) { return (char*)(v & PTR_MASK); }

// --- Thread Safe IO State ---
#define BUFFER_SIZE 4096
static pthread_mutex_t io_mutex;
static int io_initialized = 0;
static char out_buffer[BUFFER_SIZE];
static int out_buf_idx = 0;
static char in_buffer[BUFFER_SIZE];
static int in_buf_idx = 0;
static int in_buf_len = 0;

__attribute__((constructor))
void init_io_system() {
    if (!io_initialized) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&io_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        io_initialized = 1;
    }
}

void flush_buffer() {
    if (out_buf_idx > 0) {
        ssize_t written = 0;
        while (written < out_buf_idx) {
            ssize_t n = write(STDOUT_FILENO, out_buffer + written, out_buf_idx - written);
            if (n == -1) { if (errno == EINTR) continue; break; }
            written += n;
        }
        out_buf_idx = 0;
    }
}

__attribute__((destructor))
void cleanup_io() {
    if (out_buf_idx > 0) write(STDOUT_FILENO, out_buffer, out_buf_idx);
}

int fill_in_buffer() {
    flush_buffer(); 
    in_buf_idx = 0;
    in_buf_len = read(STDIN_FILENO, in_buffer, BUFFER_SIZE);
    return in_buf_len;
}

int read_char_buffered() {
    if (in_buf_idx >= in_buf_len) {
        if (fill_in_buffer() <= 0) return -1;
    }
    return (unsigned char)in_buffer[in_buf_idx++];
}

int peek_char_buffered() {
    if (in_buf_idx >= in_buf_len) {
        if (fill_in_buffer() <= 0) return -1;
    }
    return (unsigned char)in_buffer[in_buf_idx];
}

// Internal raw read (not exposed)
static char* read_line_raw() {
    pthread_mutex_lock(&io_mutex);
    size_t capacity = 128;
    size_t length = 0;
    char* buffer = (char*)malloc(capacity);
    if (!buffer) { pthread_mutex_unlock(&io_mutex); return NULL; }
    
    int c;
    while ((c = peek_char_buffered()) != -1 && c != '\n') {
        if (length + 1 >= capacity) {
            capacity *= 2;
            buffer = (char*)realloc(buffer, capacity);
        }
        buffer[length++] = (char)read_char_buffered();
    }
    buffer[length] = '\0';
    if (c == '\n') read_char_buffered();
    
    char* result = (char*)aria_alloc(length + 1);
    memcpy(result, buffer, length + 1);
    free(buffer);
    pthread_mutex_unlock(&io_mutex);
    return result;
}

// FIX: Returns Tagged String
void* input() {
    flush_buffer(); // Flush before waiting
    char* s = read_line_raw();
    return (void*)box_ptr(s, TAG_STRING);
}

// FIX: Returns Tagged String
void* read_file(void* filename_tagged) {
    char* filename = unbox_ptr((Value)filename_tagged);
    if (!filename) return (void*)0; // Null

    int fd = open(filename, O_RDONLY);
    if (fd < 0) return (void*)0;
    
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    
    char* content = (char*)aria_alloc(size + 1); 
    ssize_t total = 0;
    while (total < size) {
        ssize_t r = read(fd, content + total, size - total);
        if (r <= 0) break;
        total += r;
    }
    content[total] = '\0';
    close(fd);
    return (void*)box_ptr(content, TAG_STRING);
}

// Output Helpers (Unlocked, caller holds mutex)
void put_str_unlocked(const char* s) {
    while (*s) {
        if (out_buf_idx >= BUFFER_SIZE) flush_buffer();
        out_buffer[out_buf_idx++] = *s++;
    }
}

void put_int_unlocked(int64_t n) {
    if (n == 0) { if(out_buf_idx>=BUFFER_SIZE) flush_buffer(); out_buffer[out_buf_idx++]='0'; return; }
    if (n < 0) { if(out_buf_idx>=BUFFER_SIZE) flush_buffer(); out_buffer[out_buf_idx++]='-'; n=-n; }
    char temp[32]; int i=0;
    while(n>0) { temp[i++]=(n%10)+'0'; n/=10; }
    while(i>0) { if(out_buf_idx>=BUFFER_SIZE) flush_buffer(); out_buffer[out_buf_idx++]=temp[--i]; }
}

void put_float_unlocked(double f) {
    char temp[64]; int len = snprintf(temp, 64, "%.6f", f);
    for(int i=0; i<len; i++) { if(out_buf_idx>=BUFFER_SIZE) flush_buffer(); out_buffer[out_buf_idx++]=temp[i]; }
}

// FIX: Unbox arguments for printing
void aria_vprintf(const char* format, va_list args) {
    pthread_mutex_lock(&io_mutex);
    for (const char* p = format; *p; p++) {
        if (*p != '%') {
            if (out_buf_idx >= BUFFER_SIZE) flush_buffer();
            out_buffer[out_buf_idx++] = *p;
            continue;
        }
        p++; 
        Value v = va_arg(args, Value);
        switch (*p) {
            case 'd': 
                if ((v & TAG_BASE) == TAG_INTEGER) put_int_unlocked(unbox_int(v));
                else put_int_unlocked((int64_t)unbox_double(v)); 
                break;
            case 'f': 
                if ((v & TAG_BASE) != TAG_INTEGER) put_float_unlocked(unbox_double(v));
                else put_float_unlocked((double)unbox_int(v));
                break;
            case 's': {
                char* s = unbox_ptr(v);
                if (!s) s = "(null)";
                put_str_unlocked(s);
                break;
            }
        }
    }
    pthread_mutex_unlock(&io_mutex);
}

// Internal wrappers to match headers
void aria_printf(const char* format, ...) {
    va_list args; va_start(args, format); aria_vprintf(format, args); va_end(args);
    pthread_mutex_lock(&io_mutex); flush_buffer(); pthread_mutex_unlock(&io_mutex);
}

void print(const char* format, ...) {
    va_list args; va_start(args, format); aria_vprintf(format, args); va_end(args);
    pthread_mutex_lock(&io_mutex); flush_buffer(); pthread_mutex_unlock(&io_mutex);
}

void println(const char* s_raw) {
    // Note: s_raw usually comes from a string literal which is NOT tagged in C-land call 
    // if called from C directly. But from Aria, it's a Tagged Value. 
    // However, current codegen emits `call println` passing a tagged value in RDI.
    // So `s_raw` is actually a Value.
    Value v = (Value)s_raw;
    char* s = unbox_ptr(v);
    if (!s) s = "(null)";
    pthread_mutex_lock(&io_mutex);
    put_str_unlocked(s);
    if(out_buf_idx>=BUFFER_SIZE) flush_buffer(); out_buffer[out_buf_idx++]='\n';
    flush_buffer();
    pthread_mutex_unlock(&io_mutex);
}
