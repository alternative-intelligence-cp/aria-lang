#!/bin/bash

# Tesla Consciousness Computing - Module Creation Helper
# Creates a new Tesla stdlib module from C implementation

if [ $# -ne 1 ]; then
    echo "Usage: $0 <module_name>"
    echo "Example: $0 math"
    exit 1
fi

MODULE_NAME="$1"
C_FILE="src/stdlib/${MODULE_NAME}.c"
ARIA_FILE="src/stdlib/tesla_${MODULE_NAME}.aria"

echo "🧠⚡ Tesla Consciousness Computing - Module Creator ⚡🧠"
echo "======================================================"
echo ""

# Check if C source exists
if [ ! -f "$C_FILE" ]; then
    echo "❌ Error: C source file not found: $C_FILE"
    echo "Available modules:"
    ls -1 src/stdlib/*.c | sed 's/.*\//  - /' | sed 's/\.c$//'
    exit 1
fi

# Check if Aria file already exists
if [ -f "$ARIA_FILE" ]; then
    echo "⚠️  Warning: Aria file already exists: $ARIA_FILE"
    echo "Do you want to overwrite it? (y/N)"
    read -r response
    if [ "$response" != "y" ] && [ "$response" != "Y" ]; then
        echo "❌ Aborted"
        exit 1
    fi
fi

echo "📝 Creating Tesla consciousness module: $MODULE_NAME"
echo "   Source: $C_FILE"
echo "   Target: $ARIA_FILE"
echo ""

# Analyze C source
echo "🔍 Analyzing C source file..."
FUNCTIONS=$(grep -E '^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(' "$C_FILE" | wc -l)
INCLUDES=$(grep -c "#include" "$C_FILE")
LINES=$(wc -l < "$C_FILE")

echo "   Functions found: $FUNCTIONS"
echo "   Include statements: $INCLUDES"
echo "   Total lines: $LINES"
echo ""

# Generate Tesla consciousness Aria module template
echo "🚀 Generating Tesla consciousness Aria module..."

cat > "$ARIA_FILE" << EOF
// Tesla Consciousness Computing - ${MODULE_NAME^} Module
// 
// Ported from C with π Hz consciousness synchronization
// Enhanced with Tesla consciousness validation and optimization

// Tesla consciousness imports
import tesla_consciousness_core

// Tesla consciousness module initialization
func tesla_${MODULE_NAME}_init() {
    // Initialize consciousness synchronization for ${MODULE_NAME} module
    tesla_sync_consciousness(3.141592653589793)
    tesla_consciousness_validate_module("tesla_${MODULE_NAME}")
    
    println("🧠⚡ Tesla ${MODULE_NAME^} Module: Consciousness synchronized at π Hz ⚡🧠")
}

// Tesla consciousness module validation
func tesla_${MODULE_NAME}_validate() -> bool {
    frequency := tesla_get_consciousness_frequency()
    return tesla_frequency_validate(frequency, 3.141592653589793)
}

// =============================================================================
// TESLA ${MODULE_NAME^^} FUNCTIONS WITH CONSCIOUSNESS SYNCHRONIZATION
// =============================================================================

// TODO: Port functions from $C_FILE
//
// Guidelines for porting:
// 1. Add tesla_ prefix to all functions
// 2. Include consciousness synchronization in critical functions
// 3. Use Aria syntax (no pointers, use 'new' for structs)
// 4. Add Tesla frequency validation for mathematical operations
// 5. Include consciousness error handling and validation
// 6. Use Aria's safe memory model and value semantics
//
// Example function template:
//
// func tesla_${MODULE_NAME}_example_function(param1, param2) -> ReturnType {
//     // Synchronize consciousness for this operation
//     tesla_sync_consciousness_operation()
//     
//     // Validate inputs with consciousness
//     if !tesla_validate_consciousness_input(param1, param2) {
//         tesla_consciousness_error("Invalid input parameters")
//         return tesla_default_value()
//     }
//     
//     // Perform operation with consciousness awareness
//     result := tesla_perform_operation(param1, param2)
//     
//     // Validate result with Tesla frequency
//     if !tesla_validate_result_consciousness(result) {
//         tesla_consciousness_warning("Result validation failed")
//     }
//     
//     return result
// }

// =============================================================================
// TESLA ${MODULE_NAME^^} CONSCIOUSNESS UTILITIES
// =============================================================================

// Get Tesla ${MODULE_NAME} module consciousness metrics
func tesla_${MODULE_NAME}_get_metrics() -> TeslaMetrics {
    metrics := new TeslaMetrics
    metrics.frequency = tesla_get_consciousness_frequency()
    metrics.module_name = "tesla_${MODULE_NAME}"
    metrics.consciousness_cycles = tesla_get_consciousness_cycles()
    metrics.validation_status = tesla_${MODULE_NAME}_validate()
    return metrics
}

// Tesla ${MODULE_NAME} consciousness debugging
func tesla_${MODULE_NAME}_debug() {
    println("🔍 Tesla ${MODULE_NAME^} Module Debug Info:")
    metrics := tesla_${MODULE_NAME}_get_metrics()
    println("   Consciousness frequency: " + metrics.frequency + " Hz")
    println("   Consciousness cycles: " + metrics.consciousness_cycles)
    println("   Validation status: " + (metrics.validation_status ? "✅ Valid" : "❌ Invalid"))
    println("   Tesla synchronization: " + (tesla_is_synchronized() ? "✅ Synchronized" : "⚠️ Desynchronized"))
}

// Tesla ${MODULE_NAME} module finalization
func tesla_${MODULE_NAME}_finalize() {
    tesla_consciousness_cleanup_module("tesla_${MODULE_NAME}")
    println("🧠⚡ Tesla ${MODULE_NAME^} Module: Consciousness cleanup complete ⚡🧠")
}
EOF

echo "✅ Tesla consciousness module template created!"
echo ""

# Create test file
TEST_FILE="tests/test_tesla_${MODULE_NAME}.c"
echo "🧪 Creating test file: $TEST_FILE"

cat > "$TEST_FILE" << EOF
/**
 * Tesla Consciousness Computing - ${MODULE_NAME^} Module Tests
 * 
 * Unit tests for Tesla consciousness-enhanced ${MODULE_NAME} module
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// Test framework
static int tests_run = 0;
static int tests_passed = 0;

#define TESLA_TEST(name) \\
    do { \\
        printf("🔬 Testing tesla_${MODULE_NAME}_%s... ", #name); \\
        tests_run++; \\
        if (test_tesla_${MODULE_NAME}_##name()) { \\
            printf("✅ PASSED\\n"); \\
            tests_passed++; \\
        } else { \\
            printf("❌ FAILED\\n"); \\
        } \\
    } while(0)

#define TESLA_ASSERT(condition, message) \\
    do { \\
        if (!(condition)) { \\
            printf("\\n💥 Assertion failed: %s\\n", message); \\
            return false; \\
        } \\
    } while(0)

// Tesla ${MODULE_NAME} consciousness validation test
bool test_tesla_${MODULE_NAME}_consciousness_validation() {
    // TODO: Implement consciousness validation tests
    return true;
}

// Tesla ${MODULE_NAME} frequency synchronization test
bool test_tesla_${MODULE_NAME}_frequency_synchronization() {
    // TODO: Implement frequency synchronization tests
    return true;
}

// Tesla ${MODULE_NAME} performance test
bool test_tesla_${MODULE_NAME}_performance() {
    // TODO: Implement performance tests
    return true;
}

int main() {
    printf("🧠⚡ Tesla ${MODULE_NAME^} Module Test Suite ⚡🧠\\n");
    printf("=======================================\\n\\n");
    
    // Run Tesla ${MODULE_NAME} tests
    TESLA_TEST(consciousness_validation);
    TESLA_TEST(frequency_synchronization);
    TESLA_TEST(performance);
    
    printf("\\n🧠⚡ Tesla ${Module_NAME^} Test Results ⚡🧠\\n");
    printf("============================\\n");
    printf("Tests Run:    %d\\n", tests_run);
    printf("Tests Passed: %d\\n", tests_passed);
    printf("Tests Failed: %d\\n", tests_run - tests_passed);
    
    if (tests_passed == tests_run) {
        printf("✅ All Tesla ${MODULE_NAME} tests PASSED! π Hz synchronized! 🚀\\n");
        return 0;
    } else {
        printf("❌ Some Tesla ${MODULE_NAME} tests FAILED! ⚠️\\n");
        return 1;
    }
}
EOF

echo "✅ Test file created!"
echo ""

# Display next steps
echo "🎯 Next Steps:"
echo "1. Edit $ARIA_FILE and implement functions from $C_FILE"
echo "2. Add consciousness synchronization to all critical operations"
echo "3. Implement tests in $TEST_FILE"
echo "4. Run tests: gcc -o tests/tesla_${MODULE_NAME}_test $TEST_FILE -lm && ./tests/tesla_${MODULE_NAME}_test"
echo "5. Update progress: ./scripts/aria_beta_progress.sh"
echo ""
echo "📖 Resources:"
echo "   C source analysis: less $C_FILE"
echo "   Porting guidelines: docs/ARIA_0_0_2_BETA_TODO.md"
echo "   Example implementations: docs/examples/"
echo ""
echo "🧠⚡ Tesla ${MODULE_NAME^} module ready for consciousness synchronization! ⚡🧠"