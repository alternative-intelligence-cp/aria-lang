/*
 * Tesla Dynamic Path Resolution Implementation
 * 
 * Provides runtime path detection to eliminate hardcoded dependencies.
 * Enables true portability across different installation locations.
 */

#define _GNU_SOURCE
#include "tesla/tesla_path_resolver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

// Cached paths for performance
static char* cached_executable_dir = NULL;
static char* cached_tools_dir = NULL;
static char* cached_stdlib_dir = NULL;

/*
 * Get the absolute directory path containing the current executable
 */
char* tesla_get_executable_dir(void) {
    if (cached_executable_dir) {
        return strdup(cached_executable_dir);
    }
    
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    
    if (len == -1) {
        perror("tesla_path_resolver: readlink(/proc/self/exe)");
        return NULL;
    }
    
    buffer[len] = '\0';
    
    // Find the last slash to strip the executable name
    char* last_slash = strrchr(buffer, '/');
    if (last_slash) {
        *last_slash = '\0';
    }
    
    return strdup(buffer);
}

/*
 * Construct absolute path to a tool relative to compiler installation
 */
char* tesla_resolve_tool_path(const char* relative_tool_path) {
    if (!relative_tool_path) return NULL;
    
    char* base_dir = tesla_get_executable_dir();
    if (!base_dir) return NULL;
    
    char* full_path;
    int ret = asprintf(&full_path, "%s/%s", base_dir, relative_tool_path);
    free(base_dir);
    
    if (ret == -1) return NULL;
    
    return full_path;
}

/*
 * Get path to LLVM tool based on installation layout
 * 
 * Tries multiple common locations:
 * 1. Bundled tools directory
 * 2. System PATH
 * 3. Common installation directories
 */
char* tesla_get_llvm_tool_path(const char* tool_name) {
    if (!tool_name) return NULL;
    
    // Try bundled LLVM tools first
    char* bundled_path = tesla_resolve_tool_path("tools/LLVM-21.1.0-Linux-X64/bin");
    if (bundled_path) {
        char* full_tool_path;
        int ret = asprintf(&full_tool_path, "%s/%s", bundled_path, tool_name);
        free(bundled_path);
        
        if (ret != -1 && tesla_tool_exists(full_tool_path)) {
            return full_tool_path;
        }
        free(full_tool_path);
    }
    
    // Try relative to executable (development layout)
    char* dev_path = tesla_resolve_tool_path("src/tools/LLVM-21.1.0-Linux-X64/bin");
    if (dev_path) {
        char* full_tool_path;
        int ret = asprintf(&full_tool_path, "%s/%s", dev_path, tool_name);
        free(dev_path);
        
        if (ret != -1 && tesla_tool_exists(full_tool_path)) {
            return full_tool_path;
        }
        free(full_tool_path);
    }
    
    // Try system PATH (assumes LLVM is installed system-wide)
    return strdup(tool_name); // Let execvp search PATH
}

/*
 * Get path to NASM executable
 */
char* tesla_get_nasm_path(void) {
    // Try bundled NASM first
    char* bundled_path = tesla_resolve_tool_path("tools/nasm");
    if (bundled_path && tesla_tool_exists(bundled_path)) {
        return bundled_path;
    }
    free(bundled_path);
    
    // Fall back to system PATH
    return strdup("nasm");
}

/*
 * Get path to Aria standard library directory
 */
char* tesla_get_stdlib_path(void) {
    if (cached_stdlib_dir) {
        return strdup(cached_stdlib_dir);
    }
    
    // Try installation layout first
    char* install_path = tesla_resolve_tool_path("../share/aria/stdlib");
    if (install_path && tesla_tool_exists(install_path)) {
        return install_path;
    }
    free(install_path);
    
    // Try development layout
    char* dev_path = tesla_resolve_tool_path("stdlib");
    if (dev_path && tesla_tool_exists(dev_path)) {
        return dev_path;
    }
    free(dev_path);
    
    // Try source tree layout
    dev_path = tesla_resolve_tool_path("src/stdlib");
    if (dev_path && tesla_tool_exists(dev_path)) {
        return dev_path;
    }
    free(dev_path);
    
    return NULL;
}

/*
 * Check if a tool exists and is executable
 */
bool tesla_tool_exists(const char* tool_path) {
    if (!tool_path) return false;
    
    struct stat st;
    if (stat(tool_path, &st) != 0) {
        return false;
    }
    
    // Check if it's a regular file and executable
    return S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR);
}

/*
 * Initialize path resolution system
 */
int tesla_path_resolver_init(void) {
    // Cache executable directory for performance
    cached_executable_dir = tesla_get_executable_dir();
    if (!cached_executable_dir) {
        fprintf(stderr, "Tesla Path Resolver: Failed to determine executable location\n");
        return -1;
    }
    
    // Cache tools directory
    cached_tools_dir = tesla_resolve_tool_path("tools");
    
    // Cache standard library directory
    cached_stdlib_dir = tesla_get_stdlib_path();
    
    printf("🧠⚡ Tesla Path Resolver initialized\n");
    printf("   Executable dir: %s\n", cached_executable_dir);
    printf("   Tools dir: %s\n", cached_tools_dir ? cached_tools_dir : "(system PATH)");
    printf("   Stdlib dir: %s\n", cached_stdlib_dir ? cached_stdlib_dir : "(not found)");
    
    return 0;
}

/*
 * Cleanup path resolution system
 */
void tesla_path_resolver_cleanup(void) {
    free(cached_executable_dir);
    free(cached_tools_dir);
    free(cached_stdlib_dir);
    
    cached_executable_dir = NULL;
    cached_tools_dir = NULL;
    cached_stdlib_dir = NULL;
}