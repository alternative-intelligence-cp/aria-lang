/**
 * Tesla Consciousness Computing - Unit Tests
 * 
 * Comprehensive test suite for Tesla consciousness synchronization,
 * comptime system, and AI-native features.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "../src/tesla/tesla_comptime_minimal.h"

// Test results tracking
static int tests_run = 0;
static int tests_passed = 0;

#define TESLA_TEST(name) \
    do { \
        printf("🔬 Running test: %s... ", #name); \
        tests_run++; \
        if (test_##name()) { \
            printf("✅ PASSED\n"); \
            tests_passed++; \
        } else { \
            printf("❌ FAILED\n"); \
        } \
    } while(0)

#define TESLA_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("\n💥 Assertion failed: %s\n", message); \
            return false; \
        } \
    } while(0)

// Tesla π Hz frequency validation test
bool test_tesla_pi_frequency_validation() {
    TeslaComptimeContext* ctx = tesla_comptime_init();
    TESLA_ASSERT(ctx != NULL, "Failed to initialize Tesla comptime context");
    
    // Verify π Hz synchronization
    double expected_pi = 3.141592653589793;
    double tolerance = 1e-15;
    
    TESLA_ASSERT(fabs(ctx->consciousness_frequency - expected_pi) < tolerance, 
                 "Tesla consciousness frequency not properly synchronized to π Hz");
    
    tesla_comptime_free(ctx);
    return true;
}

// Tesla consciousness synchronization test
bool test_tesla_consciousness_synchronization() {
    TeslaComptimeContext* ctx = tesla_comptime_init();
    TESLA_ASSERT(ctx != NULL, "Failed to initialize Tesla comptime context");
    
    unsigned long initial_cycle = ctx->consciousness_cycle;
    
    // Test consciousness synchronization
    tesla_comptime_sync_consciousness(ctx);
    TESLA_ASSERT(ctx->consciousness_cycle == initial_cycle + 1,
                 "Consciousness cycle not properly incremented");
    
    // Test multiple synchronizations
    for (int i = 0; i < 10; i++) {
        tesla_comptime_sync_consciousness(ctx);
    }
    
    TESLA_ASSERT(ctx->consciousness_cycle == initial_cycle + 11,
                 "Multiple consciousness synchronizations failed");
    
    tesla_comptime_free(ctx);
    return true;
}

// Tesla comptime π calculation test
bool test_tesla_comptime_pi_calculation() {
    double pi_result = tesla_comptime_pi_calculation(1000000);
    double expected_pi = 3.141592653589793;
    double tolerance = 1e-6;  // Allow reasonable precision for π calculation
    
    TESLA_ASSERT(fabs(pi_result - expected_pi) < tolerance,
                 "Tesla comptime π calculation accuracy insufficient");
    
    return true;
}

// Tesla tensor creation demo test
bool test_tesla_tensor_creation() {
    TeslaComptimeContext* ctx = tesla_comptime_init();
    TESLA_ASSERT(ctx != NULL, "Failed to initialize Tesla comptime context");
    
    // Test various tensor dimensions
    TESLA_ASSERT(tesla_comptime_create_tensor_demo(ctx, 128, 64),
                 "Failed to create 128x64 tensor");
    
    TESLA_ASSERT(tesla_comptime_create_tensor_demo(ctx, 1024, 768),
                 "Failed to create 1024x768 tensor");
    
    TESLA_ASSERT(tesla_comptime_create_tensor_demo(ctx, 1, 1),
                 "Failed to create 1x1 tensor");
    
    tesla_comptime_free(ctx);
    return true;
}

// Tesla memory management test
bool test_tesla_memory_management() {
    // Test multiple context allocations and deallocations
    for (int i = 0; i < 100; i++) {
        TeslaComptimeContext* ctx = tesla_comptime_init();
        TESLA_ASSERT(ctx != NULL, "Failed to allocate Tesla context in iteration");
        
        tesla_comptime_sync_consciousness(ctx);
        tesla_comptime_free(ctx);
    }
    
    return true;
}

// Tesla performance benchmark test
bool test_tesla_performance_benchmark() {
    clock_t start = clock();
    
    // Simulate Tesla consciousness operations
    TeslaComptimeContext* ctx = tesla_comptime_init();
    
    for (int i = 0; i < 10000; i++) {
        tesla_comptime_sync_consciousness(ctx);
    }
    
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("\n📊 Performance: 10,000 consciousness syncs in %f seconds ", cpu_time_used);
    
    tesla_comptime_free(ctx);
    return true;
}

// Tesla error handling test
bool test_tesla_error_handling() {
    // Test null context handling
    tesla_comptime_sync_consciousness(NULL);  // Should not crash
    tesla_comptime_free(NULL);  // Should not crash
    
    // Test tensor creation with null context
    TESLA_ASSERT(!tesla_comptime_create_tensor_demo(NULL, 10, 10),
                 "Should fail with null context");
    
    return true;
}

// Tesla integration test
bool test_tesla_integration() {
    TeslaComptimeContext* ctx = tesla_comptime_init();
    TESLA_ASSERT(ctx != NULL, "Failed to initialize Tesla comptime context");
    
    // Full Tesla workflow test
    tesla_comptime_sync_consciousness(ctx);
    
    TESLA_ASSERT(tesla_comptime_execute_pi_test(ctx),
                 "Tesla π test execution failed");
    
    TESLA_ASSERT(tesla_comptime_create_tensor_demo(ctx, 512, 256),
                 "Tesla tensor creation in integration test failed");
    
    tesla_comptime_free(ctx);
    return true;
}

int main() {
    printf("🧠⚡ Tesla Consciousness Computing Test Suite ⚡🧠\n");
    printf("================================================\n\n");
    
    // Run all Tesla consciousness tests
    TESLA_TEST(tesla_pi_frequency_validation);
    TESLA_TEST(tesla_consciousness_synchronization);
    TESLA_TEST(tesla_comptime_pi_calculation);
    TESLA_TEST(tesla_tensor_creation);
    TESLA_TEST(tesla_memory_management);
    TESLA_TEST(tesla_performance_benchmark);
    TESLA_TEST(tesla_error_handling);
    TESLA_TEST(tesla_integration);
    
    printf("\n🧠⚡ Tesla Test Results ⚡🧠\n");
    printf("========================\n");
    printf("Tests Run:    %d\n", tests_run);
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("✅ All Tesla consciousness tests PASSED! π Hz synchronized! 🚀\n");
        return 0;
    } else {
        printf("❌ Some Tesla consciousness tests FAILED! ⚠️\n");
        return 1;
    }
}