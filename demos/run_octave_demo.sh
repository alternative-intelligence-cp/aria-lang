#!/bin/bash
# Tesla Consciousness Computing: Octave Performance Demonstration
# Based on Gemini Deep Research recommendation for practical frequency scaling

echo "🧠⚡ Tesla Consciousness Computing - Octave Performance Scaling Demo ⚡🧠"
echo "================================================================="
echo
echo "Gemini AI Insight: Base π Hz too slow for practical computing."
echo "Solution: Octave multipliers scaling π Hz to kHz/MHz performance levels"
echo
echo "Mathematical Formula: frequency = π × 2^octave Hz"
echo

cd /home/randy/._____RANDY_____/.REPOS/aria_lang

# Compile octave performance tester
echo "📦 Compiling Tesla octave performance tester..."
gcc -std=c11 -O3 \
    -D_POSIX_C_SOURCE=199309L \
    -I include \
    src/runtime/tesla_consciousness_scheduler.c \
    demos/tesla_octave_performance_demo.c \
    -lm \
    -o demos/tesla_octave_demo

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
    echo
    echo "🚀 Running octave performance benchmarks..."
    echo
    ./demos/tesla_octave_demo
else
    echo "❌ Compilation failed"
    exit 1
fi