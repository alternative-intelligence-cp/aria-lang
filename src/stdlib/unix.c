/* Aria_lang/src/stdlib/unix.c */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

extern void* aria_alloc(size_t size);
extern void* list_new();
extern void list_push(void* list, void* item);

typedef uint64_t Value;
#define QNAN_MASK       0x7FF8000000000000ULL
#define SIGN_BIT        0x8000000000000000ULL
#define TAG_BASE        (QNAN_MASK | SIGN_BIT)
#define TAG_INTEGER     (TAG_BASE | 4ULL)
#define TAG_STRING      (TAG_BASE | 5ULL)
#define PTR_MASK        0x0000FFFFFFFFFFFFULL

static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline Value box_ptr(void* ptr, uint64_t tag) { return tag | (uintptr_t)ptr; }
static inline int32_t unbox_int(Value v) { return (int32_t)(v & 0xFFFFFFFF); }
static inline char* unbox_str(Value v) { return (char*)(v & PTR_MASK); }

// --- Identification ---

void* unix_getuid() { return (void*)box_int(getuid()); }
void* unix_getgid() { return (void*)box_int(getgid()); }
void* unix_geteuid() { return (void*)box_int(geteuid()); }
void* unix_getegid() { return (void*)box_int(getegid()); }

// --- File Permissions ---

void* unix_chmod(void* path_tagged, void* mode_tagged) {
    char* path = unbox_str((Value)path_tagged);
    int mode = unbox_int((Value)mode_tagged); // Octal passed as decimal int usually
    if (!path) return (void*)box_int(-1);
    
    return (void*)box_int(chmod(path, (mode_t)mode));
}

void* unix_chown(void* path_tagged, void* uid_tagged, void* gid_tagged) {
    char* path = unbox_str((Value)path_tagged);
    int uid = unbox_int((Value)uid_tagged);
    int gid = unbox_int((Value)gid_tagged);
    if (!path) return (void*)box_int(-1);

    return (void*)box_int(chown(path, (uid_t)uid, (gid_t)gid));
}

// --- Process Control ---

void* unix_kill(void* pid_tagged, void* sig_tagged) {
    int pid = unbox_int((Value)pid_tagged);
    int sig = unbox_int((Value)sig_tagged);
    return (void*)box_int(kill((pid_t)pid, sig));
}

// --- IO / Pipes ---

// Returns a List [read_fd, write_fd] or NULL on error
void* unix_pipe() {
    int fds[2];
    if (pipe(fds) == -1) return (void*)0;

    void* list = list_new();
    list_push(list, (void*)box_int(fds[0]));
    list_push(list, (void*)box_int(fds[1]));
    return list;
}

void* unix_dup2(void* oldfd_tagged, void* newfd_tagged) {
    int oldfd = unbox_int((Value)oldfd_tagged);
    int newfd = unbox_int((Value)newfd_tagged);
    return (void*)box_int(dup2(oldfd, newfd));
}

void* unix_isatty(void* fd_tagged) {
    int fd = unbox_int((Value)fd_tagged);
    return (void*)box_int(isatty(fd));
}

// --- Environment ---

void* unix_setenv(void* name_tagged, void* value_tagged, void* overwrite_tagged) {
    char* name = unbox_str((Value)name_tagged);
    char* value = unbox_str((Value)value_tagged);
    int overwrite = unbox_int((Value)overwrite_tagged);
    
    if (!name || !value) return (void*)box_int(-1);
    return (void*)box_int(setenv(name, value, overwrite));
}

void* unix_unsetenv(void* name_tagged) {
    char* name = unbox_str((Value)name_tagged);
    if (!name) return (void*)box_int(-1);
    return (void*)box_int(unsetenv(name));
}
