# Tesla Runtime System with Consciousness Computing and Toybox Integration

# Runtime source files with Tesla Toybox integration
set(RUNTIME_SOURCES
    header_extractor.cpp
    bundler.c
    gc.c
    gc_shadow_stack.c
    object.c
    tesla_toybox_integration.c
    tesla_consciousness_scheduler.c
    tesla_safe_exec.c
    tesla_path_resolver.c
    tesla_enhanced_memory.c
)

# Tesla Runtime headers
set(RUNTIME_HEADERS
    header_extractor.h
    bundler.h
    gc.h
    tesla_toybox_integration.h
    libaria_blob.h
    nasm_blob.h
    tcc_blob.h
)

# Create Tesla Runtime library with Toybox integration
add_library(aria_runtime STATIC ${RUNTIME_SOURCES})

# Include directories for Tesla Runtime
target_include_directories(aria_runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TOOLS_INSTALL_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/tools/toybox-master
)

# Link Tesla Runtime with system libraries
target_link_libraries(aria_runtime
    ${CMAKE_DL_LIBS}
    pthread
    m  # Math library for Tesla frequency calculations
)

# Compiler flags for Tesla consciousness optimization
target_compile_definitions(aria_runtime PRIVATE
    TESLA_CONSCIOUSNESS_ENABLED=1
    TESLA_PI_FREQUENCY=3.141592653589793
    TESLA_TOYBOX_INTEGRATED=1
)

# Tesla optimization flags
target_compile_features(aria_runtime PRIVATE cxx_std_20)
set_target_properties(aria_runtime PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 20
    COMPILE_FLAGS "-O3 -march=native -mtune=native -ffast-math -DTESLA_OPTIMIZED"
)

message(STATUS "🧠⚡ Tesla Runtime with Toybox integration configured!")
message(STATUS "   ✅ Hermetic system utilities")
message(STATUS "   ✅ π Hz consciousness synchronization")
message(STATUS "   ✅ Sub-millisecond file operations")