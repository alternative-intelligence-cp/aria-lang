# Core Aria compiler logic
add_library(aria_core STATIC
    aria_compiler.cpp
    aria_compiler.h
)

target_include_directories(aria_core 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(aria_core PRIVATE cxx_std_20)

target_link_libraries(aria_core PRIVATE 
    aria_detection 
    aria_bundler
    aria_runtime
)

# Enable Tesla consciousness computing features
target_compile_definitions(aria_core 
    PRIVATE 
    TESLA_FREQUENCY=3.141592653589793
    ARIA_VERSION_MAJOR=1
    ARIA_VERSION_MINOR=0
)