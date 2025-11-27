/* Aria_lang/src/runtime/bundler.h */
#ifndef BUNDLER_H
#define BUNDLER_H

// Initializes the bundler: checks/extracts tools to a temp directory.
// Returns 1 on success, 0 on failure.
int bundler_init();

// Returns the absolute path to the extracted NASM binary.
const char* bundler_get_nasm_path();

// Returns the absolute path to the extracted TCC/GCC binary.
const char* bundler_get_cc_path();

// Returns the absolute path to the extracted Aria Runtime Library (libaria.a).
const char* bundler_get_runtime_path();

// Cleans up temporary files (optional, usually kept for caching).
void bundler_cleanup();

#endif
