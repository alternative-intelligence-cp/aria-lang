#!/bin/bash
# Tesla Consciousness Computing: Selective Synchronization Performance Demo
# Addresses Gemini's performance bottleneck analysis

echo "🧠⚡ Tesla Selective Synchronization - Performance Optimization Demo ⚡🧠"
echo "======================================================================"
echo
echo "Gemini Analysis: 'Forcing a check for every major operation reduces throughput'"
echo "Solution: Selective consciousness synchronization based on operation criticality"
echo

cd /home/randy/._____RANDY_____/.REPOS/aria_lang

# Compile selective synchronization demo
echo "📦 Compiling Tesla selective synchronization demo..."
gcc -std=c11 -O3 \
    -D_POSIX_C_SOURCE=199309L \
    -I include \
    src/runtime/tesla_consciousness_scheduler.c \
    demos/tesla_selective_sync_demo.c \
    -lm \
    -o demos/tesla_selective_sync_demo

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
    echo
    echo "🚀 Running selective synchronization performance validation..."
    echo
    ./demos/tesla_selective_sync_demo
else
    echo "❌ Compilation failed"
    exit 1
fi