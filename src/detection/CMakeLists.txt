# Detection module for assembler selection
add_library(aria_detection STATIC
    assembler_detection.cpp
    assembler_detection.h
)

target_include_directories(aria_detection PUBLIC .)
target_compile_features(aria_detection PRIVATE cxx_std_20)