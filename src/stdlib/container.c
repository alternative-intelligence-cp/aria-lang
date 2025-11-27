/* Aria_lang/src/stdlib/container.c */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

extern void* aria_alloc(size_t size);

typedef uint64_t Value;
#define TAG_INTEGER (0xFFF8000000000000ULL | 4ULL)
static inline Value box_int(int32_t i) { return TAG_INTEGER | (uint32_t)i; }
static inline char* unbox_str(Value v) { return (char*)(v & 0x0000FFFFFFFFFFFFULL); }

// Stack for the container child process
static char child_stack[1048576];

typedef struct {
    char* cmd;
    char* rootfs;
} ContainerArgs;

static int container_entry(void* arg) {
    ContainerArgs* args = (ContainerArgs*)arg;
    
    // 1. Remount / as private to not mess up host
    mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);
    
    // 2. Chroot into new root
    if (args->rootfs) {
        if (chroot(args->rootfs) != 0) {
            perror("chroot failed");
            return 1;
        }
        chdir("/");
        
        // Mount proc if missing
        mkdir("/proc", 0755);
        mount("proc", "/proc", "proc", 0, NULL);
    }
    
    // 3. Set hostname
    sethostname("aria-container", 14);
    
    // 4. Execute Command
    char* argv[] = { "/bin/sh", "-c", args->cmd, NULL };
    execv(argv[0], argv);
    
    perror("execv failed");
    return 1;
}

// Usage: container_run("cmd", "/path/to/rootfs")
void* container_run(void* cmd_t, void* root_t) {
    char* cmd = unbox_str((Value)cmd_t);
    char* root = unbox_str((Value)root_t);
    
    if (!cmd) return box_int(-1);

    ContainerArgs args;
    args.cmd = cmd;
    args.rootfs = root; // Can be NULL for partial isolation

    // CLONE_NEWUTS: Hostname isolation
    // CLONE_NEWPID: Process ID isolation
    // CLONE_NEWNS:  Mount isolation
    // SIGCHLD:      Notify parent on exit
    int flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD;
    
    pid_t pid = clone(container_entry, child_stack + sizeof(child_stack), flags, &args);
    
    if (pid == -1) {
        perror("clone failed");
        return box_int(-1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    return box_int(WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}

// Basic cgroups limiter (CPU shares)
void* container_limit_cpu(void* pid_t, void* shares_t) {
    int pid = (int32_t)((Value)pid_t & 0xFFFFFFFF);
    int shares = (int32_t)((Value)shares_t & 0xFFFFFFFF);
    
    char path[256];
    sprintf(path, "/sys/fs/cgroup/cpu/aria_%d", pid);
    mkdir(path, 0755);
    
    char file[512];
    sprintf(file, "%s/cpu.shares", path);
    FILE* f = fopen(file, "w");
    if(f) { fprintf(f, "%d", shares); fclose(f); }
    
    sprintf(file, "%s/cgroup.procs", path);
    f = fopen(file, "w");
    if(f) { fprintf(f, "%d", pid); fclose(f); }
    
    return box_int(1);
}
