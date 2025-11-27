/*
 * Tesla Safe Tool Execution Implementation
 * 
 * Secure replacement for system() calls using posix_spawnp.
 * Eliminates shell injection vulnerabilities and reduces overhead.
 */

#define _GNU_SOURCE
#include "tesla/tesla_safe_exec.h"
#include "tesla/tesla_consciousness_scheduler.h"
#include <spawn.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

// External environment for process execution
extern char **environ;

/*
 * Core safe execution implementation using posix_spawnp
 * 
 * This replaces the dangerous system() pattern throughout the codebase
 */
int tesla_safe_exec_tool(const char* tool_path, 
                        char* const argv[], 
                        const char* working_dir,
                        TeslaSafeExecResult* result) {
    
    // Clear result structure
    memset(result, 0, sizeof(TeslaSafeExecResult));
    
    // Validate inputs
    if (!tool_path || !argv || !result) {
        snprintf(result->error_message, sizeof(result->error_message), 
                "Invalid parameters to tesla_safe_exec_tool");
        result->execution_failed = true;
        return -1;
    }
    
    // Critical sync for secure tool execution (Gemini: Compiler operations)
    tesla_sync_selective(TESLA_SYNC_CRITICAL);
    
    pid_t pid;
    posix_spawnattr_t attr;
    posix_spawn_file_actions_t file_actions;
    int status;
    
    // Initialize spawn attributes
    if (posix_spawnattr_init(&attr) != 0) {
        snprintf(result->error_message, sizeof(result->error_message),\n                \"Failed to initialize spawn attributes: %s\", strerror(errno));\n        result->execution_failed = true;\n        return -1;\n    }\n    \n    // Initialize file actions for working directory\n    if (posix_spawn_file_actions_init(&file_actions) != 0) {\n        posix_spawnattr_destroy(&attr);\n        snprintf(result->error_message, sizeof(result->error_message),\n                \"Failed to initialize file actions: %s\", strerror(errno));\n        result->execution_failed = true;\n        return -1;\n    }\n    \n    // Set working directory if specified\n    if (working_dir) {\n        if (posix_spawn_file_actions_addchdir_np(&file_actions, working_dir) != 0) {\n            posix_spawn_file_actions_destroy(&file_actions);\n            posix_spawnattr_destroy(&attr);\n            snprintf(result->error_message, sizeof(result->error_message),\n                    \"Failed to set working directory: %s\", strerror(errno));\n            result->execution_failed = true;\n            return -1;\n        }\n    }\n    \n    // Execute the tool using posix_spawnp (searches PATH)\n    status = posix_spawnp(&pid, tool_path, &file_actions, &attr, argv, environ);\n    \n    // Clean up spawn structures\n    posix_spawn_file_actions_destroy(&file_actions);\n    posix_spawnattr_destroy(&attr);\n    \n    // Check if spawn failed\n    if (status != 0) {\n        snprintf(result->error_message, sizeof(result->error_message),\n                \"Failed to spawn %s: %s\", tool_path, strerror(status));\n        result->execution_failed = true;\n        return -1;\n    }\n    \n    // Wait for child process to complete\n    if (waitpid(pid, &status, 0) == -1) {\n        snprintf(result->error_message, sizeof(result->error_message),\n                \"Failed to wait for %s: %s\", tool_path, strerror(errno));\n        result->execution_failed = true;\n        return -1;\n    }\n    \n    // Extract exit code\n    if (WIFEXITED(status)) {\n        result->exit_code = WEXITSTATUS(status);\n        result->execution_failed = false;\n        \n        if (result->exit_code != 0) {\n            snprintf(result->error_message, sizeof(result->error_message),\n                    \"%s exited with code %d\", tool_path, result->exit_code);\n        } else {\n            snprintf(result->error_message, sizeof(result->error_message),\n                    \"%s completed successfully\", tool_path);\n        }\n    } else if (WIFSIGNALED(status)) {\n        int signal = WTERMSIG(status);\n        result->exit_code = 128 + signal; // Standard convention\n        result->execution_failed = true;\n        snprintf(result->error_message, sizeof(result->error_message),\n                \"%s terminated by signal %d\", tool_path, signal);\n    } else {\n        result->exit_code = -1;\n        result->execution_failed = true;\n        snprintf(result->error_message, sizeof(result->error_message),\n                \"%s terminated abnormally\", tool_path);\n    }\n    \n    return result->execution_failed ? -1 : 0;\n}\n\n/*\n * Execute LLVM tool with Tesla consciousness integration\n */\nint tesla_safe_exec_llvm_tool(const char* tool_name,\n                             const char* input_file,\n                             const char* output_file,\n                             ...) {\n    \n    // Build argument list dynamically\n    char* args[16]; // Maximum 16 arguments for safety\n    int argc = 0;\n    \n    // Add tool name as argv[0]\n    args[argc++] = (char*)tool_name;\n    \n    // Add input file\n    if (input_file) {\n        args[argc++] = (char*)input_file;\n    }\n    \n    // Add output file with -o flag\n    if (output_file) {\n        args[argc++] = \"-o\";\n        args[argc++] = (char*)output_file;\n    }\n    \n    // Add variable arguments\n    va_list va;\n    va_start(va, output_file);\n    char* arg;\n    while ((arg = va_arg(va, char*)) != NULL && argc < 15) {\n        args[argc++] = arg;\n    }\n    va_end(va);\n    \n    // Null-terminate argument list\n    args[argc] = NULL;\n    \n    // Execute tool\n    TeslaSafeExecResult result;\n    int ret = tesla_safe_exec_tool(tool_name, args, NULL, &result);\n    \n    if (ret != 0 || result.exit_code != 0) {\n        fprintf(stderr, \"Tesla LLVM Tool Error: %s\\n\", result.error_message);\n    }\n    \n    return result.exit_code;\n}\n\n/*\n * Execute NASM with consciousness synchronization\n */\nint tesla_safe_exec_nasm(const char* input_asm,\n                        const char* output_obj,\n                        const char* format) {\n    \n    char* args[] = {\n        \"nasm\",\n        \"-f\", (char*)(format ? format : \"elf64\"),\n        (char*)input_asm,\n        \"-o\", (char*)output_obj,\n        NULL\n    };\n    \n    TeslaSafeExecResult result;\n    int ret = tesla_safe_exec_tool(\"nasm\", args, NULL, &result);\n    \n    if (ret != 0 || result.exit_code != 0) {\n        fprintf(stderr, \"Tesla NASM Error: %s\\n\", result.error_message);\n    }\n    \n    return result.exit_code;\n}\n\n/*\n * Execute Toybox command safely\n */\nint tesla_safe_exec_toybox(const char* command, char* const args[]) {\n    // For now, this executes the system toybox binary\n    // TODO: Replace with embedded libtoybox.a integration\n    \n    TeslaSafeExecResult result;\n    int ret = tesla_safe_exec_tool(command, args, NULL, &result);\n    \n    if (ret != 0 || result.exit_code != 0) {\n        fprintf(stderr, \"Tesla Toybox Error (%s): %s\\n\", command, result.error_message);\n    }\n    \n    return result.exit_code;\n}