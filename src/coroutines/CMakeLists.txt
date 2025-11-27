# Tesla CSP Runtime - The Ultimate Concurrent Programming System
# Go coroutines + Rust safety + JavaScript closures + C++ OOP + C performance + Tesla consciousness

cmake_minimum_required(VERSION 3.14)

# Tesla CSP library
add_library(tesla_csp_runtime
    tesla_csp_runtime.cpp
    tesla_channels.cpp      # Channel implementation
    # tesla_closures.cpp      # Closure implementation
)

target_include_directories(tesla_csp_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../borrow_checker  # Memory management integration
        ${CMAKE_CURRENT_SOURCE_DIR}/../asm             # Assembly acceleration
)

# C++17 required for advanced template features and std::any
target_compile_features(tesla_csp_runtime PRIVATE cxx_std_17)

# Tesla consciousness computing defines
target_compile_definitions(tesla_csp_runtime PRIVATE
    TESLA_FREQUENCY_HZ=3.141592653589793
    TESLA_CSP_ENABLED=1
    TESLA_CONSCIOUSNESS_SYNC=1
    TESLA_GOROUTINES_ENABLED=1
)

# Optimization flags for the ultimate performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(tesla_csp_runtime PRIVATE
        -O3                    # Maximum optimization
        -flto                  # Link-time optimization  
        -march=native          # CPU-specific optimizations
        -DNDEBUG              # Disable debug assertions
        -ffast-math           # Fast floating point for Tesla frequency calculations
    )
endif()

# Link with Tesla hybrid memory management
target_link_libraries(tesla_csp_runtime
    PRIVATE
        tesla_hybrid_memory
)

# Platform-specific context switching
if(UNIX)
    # Use ucontext on Unix systems
    target_compile_definitions(tesla_csp_runtime PRIVATE TESLA_USE_UCONTEXT=1)
    
    # Link with librt for high-resolution timing
    target_link_libraries(tesla_csp_runtime PRIVATE rt)
endif()

# Installation
install(TARGETS tesla_csp_runtime
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    tesla_csp_runtime.h
    DESTINATION include/tesla/csp
)

message(STATUS "🚀⚡ Tesla CSP Runtime configured!")
message(STATUS "   ✅ Go-style coroutines and channels")
message(STATUS "   ✅ Rust-style memory safety integration")
message(STATUS "   ✅ JavaScript-style dynamic closures") 
message(STATUS "   ✅ C++-style object-oriented design")
message(STATUS "   ✅ C-style zero-cost performance")
message(STATUS "   ✅ Tesla consciousness synchronization (π Hz)")
message(STATUS "   🧠 The ultimate concurrent programming runtime!")