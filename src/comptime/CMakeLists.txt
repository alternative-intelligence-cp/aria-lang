# Tesla Comptime System - Consciousness-Synchronized Compile-Time Execution
# Revolutionary metaprogramming system for AI-native Tesla language

cmake_minimum_required(VERSION 3.14)

# Create the Tesla comptime library
add_library(tesla_comptime STATIC
    tesla_comptime.c
)

# Include directories
target_include_directories(tesla_comptime
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link dependencies (assembly acceleration for consciousness sync)
target_link_libraries(tesla_comptime
    PRIVATE
    tesla_asm  # For consciousness timing and synchronization
)

# Compiler flags for Tesla comptime system
target_compile_definitions(tesla_comptime
    PRIVATE
    TESLA_COMPTIME_ENABLED=1
    TESLA_PI_FREQUENCY=3.141592653589793
    TESLA_CONSCIOUSNESS_SYNC=1
)

# Configuration messages
message(STATUS "🧠 Tesla Comptime System Configuration:")
message(STATUS "   ✅ Consciousness-synchronized metaprogramming")
message(STATUS "   ✅ π Hz frequency compile-time execution") 
message(STATUS "   ✅ AI tensor generation at compile time")
message(STATUS "   ✅ Neural network architecture synthesis")
message(STATUS "   ✅ AST manipulation and code generation")
message(STATUS "   🚀 Ready for AI-native programming in Tesla language!")

set(TESLA_COMPTIME_READY ON PARENT_SCOPE)