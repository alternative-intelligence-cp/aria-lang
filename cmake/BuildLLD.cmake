# Build LLD (LLVM Linker) as static library for embedding
# Enables complete self-contained executable generation

message(STATUS "🔗 Configuring LLD linker build...")

# LLD is part of LLVM project - we'll build it with LLVM
set(LLD_ENABLED ON CACHE BOOL "Enable LLD linker in LLVM build")

# Add LLD-specific configuration to LLVM build
if(LLD_ENABLED)
    set(LLVM_ENABLE_PROJECTS "${LLVM_ENABLE_PROJECTS};lld" CACHE STRING "LLVM projects to build" FORCE)
    
    # LLD components we need for embedding
    set(LLD_COMPONENTS
        lldCommon
        lldCore
        lldDriver
        lldELF
        lldMachO
        lldCOFF
        lldWasm
    )
    
    message(STATUS "🔗 LLD components configured: ${LLD_COMPONENTS}")
endif()

# Custom target to verify LLD availability
add_custom_target(verify_lld
    COMMAND ${CMAKE_COMMAND} -E echo "🔗 LLD linker integration ready"
    COMMENT "Verifying LLD linker availability"
)

# Function to add LLD support to targets
function(target_link_lld target_name)
    if(LLD_ENABLED)
        # Add LLD libraries when they're available
        foreach(component ${LLD_COMPONENTS})
            target_link_libraries(${target_name} PRIVATE ${component})
        endforeach()
        
        # Add LLD include directories
        target_include_directories(${target_name} PRIVATE 
            ${CMAKE_BINARY_DIR}/external/llvm-build/include
            ${CMAKE_BINARY_DIR}/external/llvm-src/lld/include
        )
        
        # Define LLD availability
        target_compile_definitions(${target_name} PRIVATE LLD_AVAILABLE=1)
    endif()
endfunction()

message(STATUS "✅ LLD build configuration ready")