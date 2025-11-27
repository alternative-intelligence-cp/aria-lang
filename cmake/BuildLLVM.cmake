# Build LLVM as static libraries for embedding
# Based on the blueprint from selfcontainedcompiler.md

message(STATUS "🚀 Configuring LLVM build...")

# Custom target to build LLVM static libraries
add_custom_target(build_llvm_static
    COMMAND ${CMAKE_COMMAND} -E echo "🔧 Building LLVM static libraries..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LLVM_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR}
        ${CMAKE_COMMAND} -G "Unix Makefiles" ${LLVM_SOURCE_DIR}/llvm
        -DLLVM_ENABLE_PROJECTS=lld
        -DCMAKE_BUILD_TYPE=Release
        -DLLVM_TARGETS_TO_BUILD=X86
        -DBUILD_SHARED_LIBS=OFF
        -DLLVM_ENABLE_TERMINFO=OFF
        -DLLVM_ENABLE_ZLIB=OFF
        -DLLVM_ENABLE_ASSERTIONS=OFF
        -DLLVM_ENABLE_RTTI=OFF
        -DLLVM_BUILD_TOOLS=OFF
        -DLLVM_BUILD_UTILS=OFF
        -DLLVM_INCLUDE_TOOLS=OFF
        -DLLVM_INCLUDE_UTILS=OFF
        -DLLVM_INCLUDE_EXAMPLES=OFF
        -DLLVM_INCLUDE_TESTS=OFF
        -DLLVM_INCLUDE_BENCHMARKS=OFF
        -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${LLVM_BUILD_DIR} -j 8 --target lldELF lldCommon LLVMSupport LLVMCore LLVMOption
    COMMAND ${CMAKE_COMMAND} --install ${LLVM_BUILD_DIR}
    COMMENT "Building LLVM static libraries for embedding"
    VERBATIM
)

# Function to configure LLVM linking for targets
function(link_llvm_libraries target)
    # Set LLVM directory for find_package
    set(LLVM_DIR "${LLVM_INSTALL_DIR}/lib/cmake/llvm")
    
    # Find LLVM package
    find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_DIR} NO_DEFAULT_PATH)
    
    # Add LLVM definitions and include paths
    target_include_directories(${target} PRIVATE ${LLVM_INCLUDE_DIRS})
    target_compile_definitions(${target} PRIVATE ${LLVM_DEFINITIONS})
    
    # Link required LLVM libraries
    target_link_libraries(${target}
        PRIVATE
        lldELF          # Linux ELF linking
        lldCommon       # Core LLD logic
        LLVMSupport     # LLVM support utilities
        LLVMCore        # LLVM core
        LLVMOption      # Command line option parsing
        
        # System libraries often needed by LLVM static libs
        pthread
        dl
        z
    )
    
    message(STATUS "✅ LLVM libraries configured for ${target}")
endfunction()

message(STATUS "✅ LLVM build configuration ready")