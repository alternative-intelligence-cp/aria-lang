# Build NASM as static library for embedding
# Uses the "rename main" strategy from the blueprint

message(STATUS "🚀 Configuring NASM build...")

# Define NASM paths
set(NASM_VERSION "2.16.03")
set(NASM_URL "https://www.nasm.us/pub/nasm/releasebuilds/${NASM_VERSION}/nasm-${NASM_VERSION}.tar.xz")
set(NASM_SOURCE_DIR "${CMAKE_BINARY_DIR}/external/nasm-src")
set(NASM_BUILD_DIR "${CMAKE_BINARY_DIR}/external/nasm-build")

# Download NASM if needed
if(NOT EXISTS ${NASM_SOURCE_DIR}/nasm.c)
    message(STATUS "📥 Downloading NASM ${NASM_VERSION}...")
    file(DOWNLOAD ${NASM_URL} ${CMAKE_BINARY_DIR}/nasm.tar.xz)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_BINARY_DIR}/nasm.tar.xz
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/nasm-${NASM_VERSION} ${NASM_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# Custom target to build NASM static library
add_custom_target(build_nasm_static
    COMMAND ${CMAKE_COMMAND} -E echo "🔧 Building NASM static library..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${NASM_BUILD_DIR}
    
    # Copy NASM source to build directory for modification
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${NASM_SOURCE_DIR} ${NASM_BUILD_DIR}
    
    # Patch NASM main function (rename main to nasm_main_renamed)
    COMMAND sed -i 's/int main(/int nasm_main_renamed(/g' ${NASM_BUILD_DIR}/nasm.c
    
    # Configure NASM build
    COMMAND ${CMAKE_COMMAND} -E chdir ${NASM_BUILD_DIR} sh ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    
    # Build NASM objects
    COMMAND ${CMAKE_COMMAND} -E chdir ${NASM_BUILD_DIR} make -j8
    
    # Create static library from object files
    COMMAND ${CMAKE_COMMAND} -E chdir ${NASM_BUILD_DIR} ar rcs libnasm.a nasm.o || true
    
    # Install the library
    COMMAND ${CMAKE_COMMAND} -E copy ${NASM_BUILD_DIR}/libnasm.a ${CMAKE_BINARY_DIR}/
    
    COMMENT "Building NASM static library for embedding"
    VERBATIM
)

# Create NASM interface header
file(WRITE ${CMAKE_BINARY_DIR}/nasm_interface.h
"#pragma once
#ifdef __cplusplus
extern \"C\" {
#endif

// NASM entry point (renamed from main)
int nasm_main_renamed(int argc, char **argv);

#ifdef __cplusplus
}
#endif
")

# Function to add NASM support to targets
function(target_link_nasm target_name)
    add_dependencies(${target_name} build_nasm_static)
    target_link_libraries(${target_name} PRIVATE ${CMAKE_BINARY_DIR}/libnasm.a)
    target_include_directories(${target_name} PRIVATE ${CMAKE_BINARY_DIR})
endfunction()

message(STATUS "✅ NASM build configuration ready")
function(link_nasm_library target)
    # Add NASM library
    target_link_libraries(${target} PRIVATE ${LIBS_DIR}/libnasm.a)
    
    # Add NASM include directory  
    target_include_directories(${target} PRIVATE ${HEADERS_DIR}/nasm)
    
    message(STATUS "✅ NASM library configured for ${target}")
endfunction()

message(STATUS "✅ NASM build configuration ready")