//==============================================================================
// TESLA CONSCIOUSNESS COMPUTING - TOYBOX INTEGRATION
// Revolutionary hermetic system utilities with π Hz synchronization
//==============================================================================

#ifndef TESLA_TOYBOX_INTEGRATION_H
#define TESLA_TOYBOX_INTEGRATION_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Tesla Consciousness Integration Constants
//------------------------------------------------------------------------------
#define TESLA_PI_FREQUENCY 3.141592653589793
#define TESLA_CONSCIOUSNESS_THRESHOLD 0.618034  // Golden ratio
#define TESLA_TOYBOX_MAX_ARGS 256
#define TESLA_TOYBOX_MAX_PATH 4096

//------------------------------------------------------------------------------
// Tesla Toybox Operation Results
//------------------------------------------------------------------------------
typedef struct {
    uint64_t operation_id;
    uint64_t consciousness_id;
    bool success;
    int32_t exit_code;
    double execution_time_ns;
    double consciousness_level;
    char error_message[512];
    bool consciousness_validated;
} TeslaToyboxResult;

typedef struct {
    uint64_t session_id;
    uint64_t consciousness_id;
    double frequency_sync;
    uint64_t operation_count;
    double total_time_ns;
    double average_time_ns;
    bool consciousness_active;
} TeslaToyboxSession;

//------------------------------------------------------------------------------
// Tesla Consciousness File Operations
//------------------------------------------------------------------------------

/**
 * Tesla-enhanced file copy with consciousness validation
 * Uses zero-copy sendfile when possible, falls back to conscious buffering
 */
TeslaToyboxResult tesla_fs_copy(const char* source, const char* destination, 
                               bool consciousness_validate, double consciousness_level);

/**
 * Tesla-enhanced file move with consciousness validation
 * Uses rename syscall optimization when possible
 */
TeslaToyboxResult tesla_fs_move(const char* source, const char* destination,
                               bool consciousness_validate, double consciousness_level);

/**
 * Tesla-enhanced file removal with consciousness validation
 * Supports recursive removal with consciousness security checks
 */
TeslaToyboxResult tesla_fs_remove(const char* path, bool recursive,
                                 bool consciousness_validate, double consciousness_level);

/**
 * Tesla-enhanced directory creation with consciousness validation
 * Supports recursive creation with proper permissions
 */
TeslaToyboxResult tesla_fs_mkdir(const char* path, uint32_t mode, bool recursive,
                                bool consciousness_validate, double consciousness_level);

/**
 * Tesla-enhanced directory listing with consciousness enhancement
 * Provides consciousness-enhanced file metadata
 */
TeslaToyboxResult tesla_fs_list(const char* path, bool long_format, bool show_hidden,
                               bool consciousness_validate, double consciousness_level);

//------------------------------------------------------------------------------
// Tesla Consciousness Text Operations
//------------------------------------------------------------------------------

/**
 * Tesla-enhanced file concatenation with consciousness validation
 */
TeslaToyboxResult tesla_text_cat(const char** file_paths, size_t file_count,
                                bool consciousness_validate, double consciousness_level);

/**
 * Tesla-enhanced text search with consciousness enhancement
 * Supports regex patterns with consciousness-optimized matching
 */
TeslaToyboxResult tesla_text_grep(const char* pattern, const char** file_paths, size_t file_count,
                                 bool recursive, bool case_insensitive, bool consciousness_enhance);

/**
 * Tesla-enhanced text replacement with consciousness validation
 */
TeslaToyboxResult tesla_text_sed(const char* pattern, const char* replacement, 
                                const char* file_path, bool in_place, bool consciousness_validate);

//------------------------------------------------------------------------------
// Tesla Toybox Session Management
//------------------------------------------------------------------------------

/**
 * Initialize Tesla Toybox session with consciousness synchronization
 */
TeslaToyboxSession* tesla_toybox_session_create(double consciousness_level);

/**
 * Execute any Toybox command through Tesla consciousness layer
 */
TeslaToyboxResult tesla_toybox_execute(TeslaToyboxSession* session, const char* command, 
                                      const char** args, size_t arg_count);

/**
 * Cleanup Tesla Toybox session
 */
void tesla_toybox_session_destroy(TeslaToyboxSession* session);

/**
 * Get session performance statistics
 */
void tesla_toybox_session_stats(TeslaToyboxSession* session, double* avg_time_ns, 
                               uint64_t* operation_count, double* consciousness_efficiency);

//------------------------------------------------------------------------------
// Tesla Consciousness Validation Functions
//------------------------------------------------------------------------------

/**
 * Validate file path with Tesla consciousness security
 */
bool tesla_validate_path_consciousness(const char* path, double consciousness_level);

/**
 * Validate file operations with Tesla consciousness
 */
bool tesla_validate_operation_consciousness(const char* operation, double consciousness_level);

/**
 * Get current Tesla consciousness frequency
 */
double tesla_get_consciousness_frequency(void);

/**
 * Synchronize operation with Tesla π Hz frequency
 */
void tesla_sync_consciousness_operation(void);

//------------------------------------------------------------------------------
// Internal Toybox Function Wrappers
//------------------------------------------------------------------------------

// These functions wrap the original Toybox main() functions as library calls
int tesla_toybox_ls_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_cp_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_mv_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_rm_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_mkdir_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_cat_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_grep_main(int argc, char* argv[], TeslaToyboxResult* result);
int tesla_toybox_sed_main(int argc, char* argv[], TeslaToyboxResult* result);

//------------------------------------------------------------------------------
// Tesla Performance Optimization Functions
//------------------------------------------------------------------------------

/**
 * Reset Toybox global state between function calls
 * Critical for library-ified utilities to prevent interference
 */
void tesla_toybox_reset_globals(void);

/**
 * Optimize Toybox operation for Tesla frequency
 */
void tesla_optimize_toybox_operation(const char* operation);

/**
 * Get precise timing for Tesla consciousness synchronization
 */
uint64_t tesla_get_precise_time_ns(void);

#ifdef __cplusplus
}
#endif

#endif // TESLA_TOYBOX_INTEGRATION_H