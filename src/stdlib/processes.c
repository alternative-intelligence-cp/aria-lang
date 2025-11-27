/* Aria_lang/src/stdlib/processes.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>

extern void* aria_alloc(size_t size);

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

void* proc_exec(void* cmd_tagged) {
    char* cmd = unbox_str((Value)cmd_tagged);
    if (!cmd) return (void*)box_int(-1);
    return (void*)box_int(system(cmd));
}

void* proc_read(void* cmd_tagged) {
    char* cmd = unbox_str((Value)cmd_tagged);
    if (!cmd) return NULL;
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    size_t cap = 1024;
    size_t len = 0;
    char* buf = malloc(cap);
    if (!buf) { pclose(fp); return NULL; }
    
    char tmp[1024];
    while (fgets(tmp, sizeof(tmp), fp)) {
        size_t l = strlen(tmp);
        if (len + l >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        memcpy(buf + len, tmp, l);
        len += l;
    }
    buf[len] = '\0';
    pclose(fp);
    
    char* res = (char*)aria_alloc(len + 1);
    memcpy(res, buf, len + 1);
    free(buf);
    return (void*)box_ptr(res, TAG_STRING);
}

void* proc_fork() { return (void*)box_int(fork()); }
void* proc_pid() { return (void*)box_int(getpid()); }
void proc_wait(void* pid_tagged) { 
    int pid = unbox_int((Value)pid_tagged);
    waitpid(pid, NULL, 0); 
}
void proc_exit(void* code_tagged) { 
    exit(unbox_int((Value)code_tagged)); 
}
