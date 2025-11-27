/*
 * Tesla Safe Tool Execution System
 * 
 * Replaces dangerous system() calls with secure posix_spawnp implementation.
 * 
 * Security Benefits:
 * - Eliminates shell injection vulnerabilities
 * - No shell interpreter involvement  
 * - Direct process execution
 * - Proper error handling and exit codes
 * 
 * Performance Benefits:
 * - Lower overhead than fork+exec
 * - Faster process creation
 * - Better resource management
 */

#ifndef TESLA_SAFE_EXEC_H
#define TESLA_SAFE_EXEC_H

#include <stddef.h>
#include <stdbool.h>

/*
 * Tesla Safe Tool Execution Result
 */
typedef struct {
    int exit_code;           // Process exit code (0 = success)
    bool execution_failed;   // True if exec failed (vs process ran and exited with error)
    char error_message[256]; // Human-readable error description
} TeslaSafeExecResult;

/*
 * Execute a tool safely without shell involvement
 * 
 * @param tool_path Path to executable (can be relative name for PATH lookup)
 * @param argv Null-terminated argument array (argv[0] should be program name)
 * @param working_dir Working directory for execution (NULL = inherit)
 * @param result Output result structure
 * @return 0 on successful execution, -1 on setup failure
 */
int tesla_safe_exec_tool(const char* tool_path, 
                        char* const argv[], 
                        const char* working_dir,
                        TeslaSafeExecResult* result);

/*
 * Execute LLVM tool with standard Tesla consciousness integration
 * 
 * @param tool_name LLVM tool name (e.g., "llvm-mc", "llc")
 * @param input_file Input file path
 * @param output_file Output file path
 * @param extra_args Additional arguments (NULL to end list)
 * @return 0 on success, non-zero on failure
 */
int tesla_safe_exec_llvm_tool(const char* tool_name,
                             const char* input_file,
                             const char* output_file,
                             ...);

/*
 * Execute NASM with consciousness synchronization
 * 
 * @param input_asm Assembly source file
 * @param output_obj Output object file
 * @param format Output format (e.g., "elf64")
 * @return 0 on success, non-zero on failure
 */
int tesla_safe_exec_nasm(const char* input_asm,
                        const char* output_obj,
                        const char* format);

/*
 * Execute Toybox command safely
 * 
 * @param command Toybox command name (e.g., "ls", "cp", "mv")
 * @param args Command arguments
 * @return 0 on success, non-zero on failure
 */
int tesla_safe_exec_toybox(const char* command, char* const args[]);

#endif /* TESLA_SAFE_EXEC_H */