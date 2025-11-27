# Tesla Native Meta-Assembly Preprocessor
# Replaces NASM dependency with native Aria preprocessing

message(STATUS "🔮 Building Tesla Native Preprocessor...")

# Tesla Preprocessor - Phase 1: Advanced Macro System
add_library(tesla_preprocessor STATIC
    tesla_preprocessor.cpp
    tesla_preprocessor.h
    macro_engine.cpp
    macro_engine.h
    consciousness_directives.cpp
    consciousness_directives.h
)

target_include_directories(tesla_preprocessor PUBLIC .)
target_compile_features(tesla_preprocessor PRIVATE cxx_std_20)

# Tesla Preprocessor features
target_compile_definitions(tesla_preprocessor PRIVATE
    TESLA_NATIVE_PREPROCESSING=1
    TESLA_FREQUENCY=3.141592653589793
    CONSCIOUSNESS_COMPUTING=1
)

message(STATUS "✅ Tesla Native Preprocessor configured")