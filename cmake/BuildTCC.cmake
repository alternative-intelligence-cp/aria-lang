# Build TinyCC as static library for embedding
# Uses libtcc API as described in the blueprint

message(STATUS "🚀 Configuring TCC build...")

# Custom target to build TCC static library
add_custom_target(build_tcc_static
    COMMAND ${CMAKE_COMMAND} -E echo "🔧 Building TCC static library..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TCC_BUILD_DIR}
    
    # Copy TCC source to build directory
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TCC_SOURCE_DIR} ${TCC_BUILD_DIR}
    
    # Configure TCC build for static library
    COMMAND ${CMAKE_COMMAND} -E chdir ${TCC_BUILD_DIR}
        ./configure 
        --prefix=${TOOLS_INSTALL_DIR}
        --enable-static
        --disable-shared
        --config-musl
    
    # Build TCC
    COMMAND ${CMAKE_COMMAND} -E chdir ${TCC_BUILD_DIR}
        make -j 8 libtcc.a
    
    # Install library and headers
    COMMAND ${CMAKE_COMMAND} -E copy ${TCC_BUILD_DIR}/libtcc.a ${LIBS_DIR}/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HEADERS_DIR}/tcc
    COMMAND ${CMAKE_COMMAND} -E copy ${TCC_BUILD_DIR}/libtcc.h ${HEADERS_DIR}/tcc/
    
    # Copy TCC's own include files (stdio.h, stdlib.h, etc.)
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TCC_BUILD_DIR}/include ${HEADERS_DIR}/tcc_includes
    
    COMMENT "Building TCC static library for embedding"
    VERBATIM
)

# Function to link TCC library to targets
function(link_tcc_library target)
    # Add TCC library
    target_link_libraries(${target} PRIVATE ${LIBS_DIR}/libtcc.a)
    
    # Add TCC include directory
    target_include_directories(${target} PRIVATE ${HEADERS_DIR}/tcc)
    
    # TCC often needs these system libraries
    target_link_libraries(${target} PRIVATE dl m)
    
    message(STATUS "✅ TCC library configured for ${target}")
endfunction()

message(STATUS "✅ TCC build configuration ready")