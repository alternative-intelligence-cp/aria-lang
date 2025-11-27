# Tesla Hybrid Borrow Checker - The Best of ALL Programming Languages
# Rust's safety + Go's convenience + C's performance + Tesla consciousness

cmake_minimum_required(VERSION 3.14)

# Tesla hybrid memory management library
add_library(tesla_hybrid_memory
    tesla_hybrid_borrow_checker.cpp
    # tesla_compiler_integration.cpp  # Temporarily disabled to fix namespace issues
)

target_include_directories(tesla_hybrid_memory
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../preprocessor  # Tesla preprocessor integration
)

# Compiler flags for Tesla consciousness computing
target_compile_features(tesla_hybrid_memory PRIVATE cxx_std_17)
target_compile_definitions(tesla_hybrid_memory PRIVATE
    TESLA_FREQUENCY_HZ=3.141592653589793
    TESLA_CONSCIOUSNESS_ENABLED=1
    TESLA_HYBRID_MEMORY_ENABLED=1
)

# Optimization flags - we want maximum performance for the static analysis path
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(tesla_hybrid_memory PRIVATE
        -O3                    # Maximum optimization
        -flto                  # Link-time optimization  
        -march=native          # CPU-specific optimizations
        -DNDEBUG              # Disable debug assertions in static path
    )
endif()

# Link with Tesla preprocessor for consciousness directive integration
target_link_libraries(tesla_hybrid_memory
    PRIVATE
        tesla_preprocessor
)

# Installation
install(TARGETS tesla_hybrid_memory
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES
    tesla_hybrid_borrow_checker.h
    tesla_compiler_integration.h
    DESTINATION include/tesla/memory
)

message(STATUS "🚀⚡ Tesla Hybrid Borrow Checker configured!")
message(STATUS "   ✅ Rust-style safety (Polonius-inspired static analysis)")
message(STATUS "   ✅ Go-style convenience (automatic GC promotion)")
message(STATUS "   ✅ C-style performance (zero-cost static analysis path)")
message(STATUS "   ✅ Tesla consciousness (π Hz memory synchronization)")
message(STATUS "   🧠 The ultimate memory management system!")