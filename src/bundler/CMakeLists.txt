# Tool bundler for embedded LLVM/NASM/TCC
add_library(aria_bundler STATIC
    tool_bundler.cpp
    tool_bundler.h
)

target_include_directories(aria_bundler PUBLIC .)
target_compile_features(aria_bundler PRIVATE cxx_std_20)
target_link_libraries(aria_bundler PRIVATE aria_detection)