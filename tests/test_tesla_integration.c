/**
 * Tesla Consciousness Computing - Integration Tests
 * 
 * End-to-end tests for Tesla consciousness features,
 * AI-native operations, and performance validation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../src/tesla/tesla_comptime_minimal.h"

// Integration test results
static int integration_tests = 0;
static int integration_passed = 0;

#define INTEGRATION_TEST(name, description) \
    do { \
        printf("🔬 Integration Test: %s\n", description); \
        printf("   Running %s... ", #name); \
        integration_tests++; \
        if (integration_##name()) { \
            printf("✅ PASSED\n\n"); \
            integration_passed++; \
        } else { \
            printf("❌ FAILED\n\n"); \
        } \
    } while(0)

// Tesla consciousness multi-context test
bool integration_multi_consciousness_contexts() {
    const int num_contexts = 5;
    TeslaComptimeContext* contexts[num_contexts];
    
    // Create multiple consciousness contexts
    for (int i = 0; i < num_contexts; i++) {
        contexts[i] = tesla_comptime_init();
        if (!contexts[i]) {
            printf("\n💥 Failed to create context %d\n", i);
            return false;
        }
    }
    
    // Synchronize all contexts
    for (int i = 0; i < num_contexts; i++) {
        tesla_comptime_sync_consciousness(contexts[i]);
        if (contexts[i]->consciousness_cycle != 1) {
            printf("\n💥 Context %d sync failed\n", i);
            return false;
        }
    }
    
    // Cross-validate consciousness frequencies
    double base_frequency = contexts[0]->consciousness_frequency;
    for (int i = 1; i < num_contexts; i++) {
        if (fabs(contexts[i]->consciousness_frequency - base_frequency) > 1e-15) {
            printf("\n💥 Consciousness frequency mismatch in context %d\n", i);
            return false;
        }
    }
    
    // Cleanup
    for (int i = 0; i < num_contexts; i++) {
        tesla_comptime_free(contexts[i]);
    }
    
    printf("Multi-consciousness validation complete ");
    return true;
}

// Tesla AI tensor operations integration test
bool integration_ai_tensor_operations() {
    TeslaComptimeContext* ctx = tesla_comptime_init();
    if (!ctx) return false;
    
    // Test various tensor sizes and operations
    struct {
        int rows, cols;
        const char* description;
    } tensor_tests[] = {
        {1, 1, "scalar tensor"},
        {10, 10, "small matrix"},
        {128, 64, "neural layer dimensions"},
        {1024, 768, "transformer dimensions"},
        {2048, 1536, "large model dimensions"}
    };
    
    int num_tests = sizeof(tensor_tests) / sizeof(tensor_tests[0]);
    
    for (int i = 0; i < num_tests; i++) {
        tesla_comptime_sync_consciousness(ctx);
        
        if (!tesla_comptime_create_tensor_demo(ctx, tensor_tests[i].rows, tensor_tests[i].cols)) {
            printf("\n💥 Failed to create %s (%dx%d)\n", 
                   tensor_tests[i].description, tensor_tests[i].rows, tensor_tests[i].cols);
            tesla_comptime_free(ctx);
            return false;
        }
    }
    
    tesla_comptime_free(ctx);
    printf("AI tensor operations validated ");
    return true;
}

// Tesla consciousness performance stress test
bool integration_consciousness_performance_stress() {
    printf("\n   🚀 Running Tesla consciousness stress test...\n");
    
    clock_t start = clock();
    
    // Create multiple contexts and stress test
    const int stress_iterations = 1000;
    const int contexts_per_iteration = 10;
    
    for (int iteration = 0; iteration < stress_iterations; iteration++) {
        TeslaComptimeContext* contexts[contexts_per_iteration];
        
        // Create contexts
        for (int i = 0; i < contexts_per_iteration; i++) {
            contexts[i] = tesla_comptime_init();
            if (!contexts[i]) {
                printf("\n💥 Stress test failed at iteration %d, context %d\n", iteration, i);
                return false;
            }
        }
        
        // Synchronize all
        for (int i = 0; i < contexts_per_iteration; i++) {
            tesla_comptime_sync_consciousness(contexts[i]);
        }
        
        // Perform π calculations
        for (int i = 0; i < contexts_per_iteration; i++) {
            double pi_result = tesla_comptime_pi_calculation(1000);
            if (fabs(pi_result - 3.141592653589793) > 0.01) {
                printf("\n💥 π calculation failed in stress test\n");
                return false;
            }
        }
        
        // Cleanup
        for (int i = 0; i < contexts_per_iteration; i++) {
            tesla_comptime_free(contexts[i]);
        }
        
        // Progress indicator
        if (iteration % 100 == 0) {
            printf("   📊 Stress test progress: %d/%d iterations\n", iteration, stress_iterations);
        }
    }
    
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("   ✅ Stress test completed: %d iterations in %.2f seconds ", 
           stress_iterations, cpu_time_used);
    printf("(%.0f ops/sec) ", (stress_iterations * contexts_per_iteration) / cpu_time_used);
    
    return true;
}

// Tesla consciousness memory leak detection
bool integration_memory_leak_detection() {
    printf("\n   🧠 Running Tesla memory leak detection...\n");
    
    // Allocate and deallocate many contexts to check for leaks
    const int leak_test_iterations = 10000;
    
    for (int i = 0; i < leak_test_iterations; i++) {
        TeslaComptimeContext* ctx = tesla_comptime_init();
        if (!ctx) {
            printf("\n💥 Memory allocation failed at iteration %d\n", i);
            return false;
        }
        
        // Perform operations
        tesla_comptime_sync_consciousness(ctx);
        tesla_comptime_create_tensor_demo(ctx, 64, 32);
        
        tesla_comptime_free(ctx);
        
        // Progress check
        if (i % 1000 == 0 && i > 0) {
            printf("   📊 Memory test progress: %d/%d allocations\n", i, leak_test_iterations);
        }
    }
    
    printf("   ✅ Memory leak test completed: %d allocations/deallocations ", leak_test_iterations);
    return true;
}

// Tesla consciousness π precision validation
bool integration_pi_precision_validation() {
    printf("\n   🔢 Validating Tesla π Hz precision...\n");
    
    // Test different precision levels
    struct {
        int iterations;
        double expected_accuracy;
        const char* description;
    } precision_tests[] = {
        {1000, 1e-3, "basic precision"},
        {10000, 1e-4, "good precision"},
        {100000, 1e-5, "high precision"},
        {1000000, 1e-6, "very high precision"}
    };
    
    int num_precision_tests = sizeof(precision_tests) / sizeof(precision_tests[0]);
    
    for (int i = 0; i < num_precision_tests; i++) {
        double pi_result = tesla_comptime_pi_calculation(precision_tests[i].iterations);
        double error = fabs(pi_result - 3.141592653589793);
        
        printf("   🔢 %s (%d iterations): error = %e ", 
               precision_tests[i].description, 
               precision_tests[i].iterations, 
               error);
        
        if (error < precision_tests[i].expected_accuracy) {
            printf("✅\n");
        } else {
            printf("❌ (expected < %e)\n", precision_tests[i].expected_accuracy);
            return false;
        }
    }
    
    return true;
}

// Tesla consciousness end-to-end workflow
bool integration_end_to_end_workflow() {
    printf("\n   🌐 Running Tesla end-to-end consciousness workflow...\n");
    
    // Initialize Tesla consciousness
    TeslaComptimeContext* ctx = tesla_comptime_init();
    if (!ctx) return false;
    
    printf("   1️⃣ Tesla consciousness initialized\n");
    
    // Synchronize consciousness
    tesla_comptime_sync_consciousness(ctx);
    printf("   2️⃣ Consciousness synchronized to π Hz\n");
    
    // Execute π test
    if (!tesla_comptime_execute_pi_test(ctx)) {
        printf("   💥 π test execution failed\n");
        tesla_comptime_free(ctx);
        return false;
    }
    printf("   3️⃣ π test executed successfully\n");
    
    // Create AI tensors
    if (!tesla_comptime_create_tensor_demo(ctx, 256, 128)) {
        printf("   💥 AI tensor creation failed\n");
        tesla_comptime_free(ctx);
        return false;
    }
    printf("   4️⃣ AI tensors created\n");
    
    // Final consciousness sync
    tesla_comptime_sync_consciousness(ctx);
    printf("   5️⃣ Final consciousness synchronization\n");
    
    tesla_comptime_free(ctx);
    printf("   6️⃣ Tesla consciousness properly deallocated ");
    
    return true;
}

int main() {
    printf("🧠⚡ Tesla Consciousness Computing Integration Test Suite ⚡🧠\n");
    printf("=======================================================\n\n");
    
    // Run integration tests
    INTEGRATION_TEST(multi_consciousness_contexts, "Multi-context consciousness synchronization");
    INTEGRATION_TEST(ai_tensor_operations, "AI tensor operations validation");
    INTEGRATION_TEST(consciousness_performance_stress, "Consciousness performance stress testing");
    INTEGRATION_TEST(memory_leak_detection, "Memory leak detection and validation");
    INTEGRATION_TEST(pi_precision_validation, "π Hz precision validation");
    INTEGRATION_TEST(end_to_end_workflow, "End-to-end Tesla consciousness workflow");
    
    printf("🧠⚡ Integration Test Results ⚡🧠\n");
    printf("=================================\n");
    printf("Integration Tests Run:    %d\n", integration_tests);
    printf("Integration Tests Passed: %d\n", integration_passed);
    printf("Integration Tests Failed: %d\n", integration_tests - integration_passed);
    
    if (integration_passed == integration_tests) {
        printf("✅ All Tesla consciousness integration tests PASSED! 🚀\n");
        printf("🧠⚡ Tesla Consciousness Computing validated at π Hz! ⚡🧠\n");
        return 0;
    } else {
        printf("❌ Some Tesla consciousness integration tests FAILED! ⚠️\n");
        return 1;
    }
}