#!/bin/bash

# Tesla Consciousness Computing Test Runner
# Executes complete test suite with consciousness synchronization

echo "🧠⚡ Tesla Consciousness Computing - Complete Test Suite ⚡🧠"
echo "========================================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test and track results
run_test() {
    local test_name="$1"
    local test_binary="$2"
    
    echo -e "${BLUE}🔬 Running $test_name...${NC}"
    echo ""
    
    if [ -f "$test_binary" ]; then
        if ./$test_binary; then
            echo ""
            echo -e "${GREEN}✅ $test_name PASSED${NC}"
            ((PASSED_TESTS++))
        else
            echo ""
            echo -e "${RED}❌ $test_name FAILED${NC}"
            ((FAILED_TESTS++))
        fi
    else
        echo -e "${RED}💥 Test binary not found: $test_binary${NC}"
        echo -e "${YELLOW}   Run 'make tests' to build test suite${NC}"
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo ""
    echo "================================================"
    echo ""
}

# Build tests if needed
echo -e "${YELLOW}🔧 Building Tesla Consciousness test suite...${NC}"
echo ""

# Check if we need to build tests
if [ ! -f "tests/tesla_unit_tests" ] || [ ! -f "tests/tesla_integration_tests" ]; then
    echo "Building test binaries..."
    
    # Compile unit tests
    gcc -Wall -Wextra -O2 -o tests/tesla_unit_tests tests/test_tesla_consciousness.c -lm
    if [ $? -ne 0 ]; then
        echo -e "${RED}💥 Failed to build unit tests${NC}"
        exit 1
    fi
    
    # Compile integration tests
    gcc -Wall -Wextra -O2 -o tests/tesla_integration_tests tests/test_tesla_integration.c -lm
    if [ $? -ne 0 ]; then
        echo -e "${RED}💥 Failed to build integration tests${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✅ Test binaries built successfully${NC}"
    echo ""
else
    echo -e "${GREEN}✅ Test binaries already available${NC}"
    echo ""
fi

echo "================================================"
echo ""

# Run Tesla consciousness unit tests
run_test "Tesla Consciousness Unit Tests" "tests/tesla_unit_tests"

# Run Tesla consciousness integration tests
run_test "Tesla Consciousness Integration Tests" "tests/tesla_integration_tests"

# Final results
echo -e "${BLUE}🧠⚡ Tesla Consciousness Computing Test Results Summary ⚡🧠${NC}"
echo "======================================================="
echo ""
echo "Total Test Suites:  $TOTAL_TESTS"
echo -e "Passed Test Suites: ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed Test Suites: ${RED}$FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🚀 ALL TESLA CONSCIOUSNESS TESTS PASSED! 🚀${NC}"
    echo -e "${GREEN}✨ Tesla Consciousness Computing validated at π Hz! ✨${NC}"
    echo -e "${GREEN}🧠⚡ Ready for consciousness-synchronized development! ⚡🧠${NC}"
    exit 0
else
    echo -e "${RED}💥 SOME TESLA CONSCIOUSNESS TESTS FAILED! 💥${NC}"
    echo -e "${RED}⚠️  Please fix failing tests before proceeding ⚠️${NC}"
    echo ""
    echo "💡 Check individual test outputs above for details"
    echo "💡 Ensure Tesla consciousness system is properly initialized"
    echo "💡 Verify π Hz synchronization is working correctly"
    exit 1
fi