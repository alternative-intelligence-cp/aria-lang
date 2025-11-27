# Runtime header extraction system
add_library(aria_runtime STATIC
    header_extractor.cpp
    header_extractor.h
)

target_include_directories(aria_runtime PUBLIC .)
target_compile_features(aria_runtime PRIVATE cxx_std_20)