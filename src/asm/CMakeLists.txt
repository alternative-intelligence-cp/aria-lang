# Tesla Assembly Acceleration Layer
# Ultra-optimized assembly routines for maximum performance
# Sub-5ns context switching, REP MOVSB memory ops, SIMD strings, π Hz timing

cmake_minimum_required(VERSION 3.14)

# Enable assembly language
enable_language(ASM)

# Tesla Assembly Library
add_library(tesla_asm STATIC
    # Context switching - Sub-5ns coroutine performance
    tesla_context_switch.S
    
    # Memory operations - REP MOVSB + AVX optimization
    tesla_memory.S
    
    # String processing - SIMD acceleration with PCMPISTRI
    tesla_strings.S
    
    # Timing and synchronization - RDTSC/RDTSCP precision
    tesla_timing.S
)

# Set assembly properties
set_property(SOURCE tesla_context_switch.S PROPERTY LANGUAGE ASM)
set_property(SOURCE tesla_memory.S PROPERTY LANGUAGE ASM)
set_property(SOURCE tesla_strings.S PROPERTY LANGUAGE ASM)
set_property(SOURCE tesla_timing.S PROPERTY LANGUAGE ASM)

# Include directories
target_include_directories(tesla_asm
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Assembly flags for optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ASM_FLAGS "-O2")
else()
    set(ASM_FLAGS "-g")
endif()

# Apply assembly flags
set_target_properties(tesla_asm PROPERTIES
    ASM_FLAGS "${ASM_FLAGS}"
    POSITION_INDEPENDENT_CODE ON
)

# Platform-specific optimizations
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    # x86-64 specific optimizations
    target_compile_definitions(tesla_asm PRIVATE
        TESLA_X86_64=1
        TESLA_SSE42=1
        TESLA_AVX2=1
    )
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    # ARM64 specific optimizations (future)
    target_compile_definitions(tesla_asm PRIVATE
        TESLA_ARM64=1
        TESLA_NEON=1
    )
endif()

# Link with math library for floating point constants
target_link_libraries(tesla_asm PRIVATE m)

# Generate assembly listings for debugging (optional)
if(TESLA_GENERATE_ASM_LISTINGS)
    add_custom_command(TARGET tesla_asm POST_BUILD
        COMMAND objdump -d $<TARGET_FILE:tesla_asm> > tesla_asm_disassembly.txt
        COMMENT "Generating Tesla assembly disassembly listing"
        VERBATIM
    )
endif()

# Performance testing target
if(TESLA_BUILD_ASM_TESTS)
    add_executable(tesla_asm_benchmark
        test_tesla_asm_performance.c
    )
    
    target_link_libraries(tesla_asm_benchmark
        tesla_asm
        ${CMAKE_THREAD_LIBS_INIT}
    )
    
    target_include_directories(tesla_asm_benchmark PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()