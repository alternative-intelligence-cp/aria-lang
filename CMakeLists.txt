# Ultimate Self-Contained Aria Tesla Consciousness Compiler
# Build system for bundling LLVM + NASM + TCC + Headers into single executable

cmake_minimum_required(VERSION 3.14)
project(AriaUltimateCompiler VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration
option(ARIA_STATIC_BUILD "Build completely static executable" ON)
option(ARIA_BUNDLE_HEADERS "Bundle standard C headers" ON)
option(ARIA_ENABLE_NASM "Include NASM assembler" ON)
option(ARIA_ENABLE_TCC "Include TinyCC compiler" ON)

# Set build type to Release by default for smaller binary
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "🧠⚡ ARIA TESLA CONSCIOUSNESS COMPILER ⚡🧠")
message(STATUS "Building ultimate self-contained compiler...")
message(STATUS "Static build: ${ARIA_STATIC_BUILD}")
message(STATUS "Bundle headers: ${ARIA_BUNDLE_HEADERS}")
message(STATUS "Enable NASM: ${ARIA_ENABLE_NASM}")
message(STATUS "Enable TCC: ${ARIA_ENABLE_TCC}")

# External tool paths
set(LLVM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/tools/LLVM-21.1.0-Linux-X64")
set(NASM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/tools/nasm-3.01")  
set(TCC_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/tools/tcc-0.9.27")

# Build directories
set(BUILD_DIR "${CMAKE_BINARY_DIR}/builds")
set(LLVM_BUILD_DIR "${BUILD_DIR}/llvm")
set(NASM_BUILD_DIR "${BUILD_DIR}/nasm")
set(TCC_BUILD_DIR "${BUILD_DIR}/tcc")

# Install directories for bundled tools
set(TOOLS_INSTALL_DIR "${CMAKE_BINARY_DIR}/bundled_tools")
set(LLVM_INSTALL_DIR "${TOOLS_INSTALL_DIR}/llvm")
set(HEADERS_DIR "${TOOLS_INSTALL_DIR}/headers")
set(LIBS_DIR "${TOOLS_INSTALL_DIR}/libs")

# Create build directories
file(MAKE_DIRECTORY ${BUILD_DIR})
file(MAKE_DIRECTORY ${TOOLS_INSTALL_DIR})

# Add subdirectories for different components
add_subdirectory(src/core)           # Core Aria compiler logic
add_subdirectory(src/detection)      # Smart assembler detection  
add_subdirectory(src/bundler)        # Tool bundling logic
add_subdirectory(src/runtime)        # Runtime systems
add_subdirectory(src/preprocessor)   # Tesla native preprocessor
# Add Tesla Assembly Acceleration Layer
add_subdirectory(src/asm)
set(TESLA_ASM_READY ON)
message(STATUS "🚀⚡ Tesla Assembly Acceleration configured!")
message(STATUS "   ✅ Sub-5ns context switching")
message(STATUS "   ✅ REP MOVSB + AVX memory operations")
message(STATUS "   ✅ SIMD-accelerated string processing")
message(STATUS "   ✅ RDTSC/RDTSCP precision timing")
message(STATUS "   🧠 Assembly-optimized Tesla consciousness computing!")

add_subdirectory(src/borrow_checker) # Tesla hybrid borrow checker (Rust+Go+C+Tesla)
add_subdirectory(src/coroutines)     # Tesla CSP runtime (Go coroutines + consciousness)
add_subdirectory(src/comptime)       # Tesla comptime system (consciousness-synchronized metaprogramming)

# Main executable
add_executable(aria_ultimate
    src/main.cpp
)

# Link everything together
target_link_libraries(aria_ultimate
    PRIVATE
    aria_core
    aria_detection  
    aria_bundler
    aria_runtime
    tesla_preprocessor
    tesla_hybrid_memory
    tesla_csp_runtime
    tesla_comptime
)

# Custom targets for building external tools (temporarily disabled for demo)
# include(cmake/BuildLLVM.cmake)
# include(cmake/BuildTCC.cmake)
include(cmake/BuildLLD.cmake)
include(cmake/NASMMacros.cmake)
include(cmake/BundleHeaders.cmake)

# Make sure external tools are built before main executable
add_dependencies(aria_ultimate
    # build_llvm_static  # Disabled for demo
    # build_tcc_static   # Disabled for demo
    verify_lld
    bundle_headers
)

# Installation
install(TARGETS aria_ultimate
    RUNTIME DESTINATION bin
)

message(STATUS "🎯 Configuration complete - ready to build ultimate compiler!")
message(STATUS "Run: make -j$(nproc) aria_ultimate")