# Bundle standard C headers as embedded byte arrays
# Uses the "embed and extract" strategy from the blueprint

message(STATUS "🚀 Configuring header bundling...")

# Create script to embed headers as C arrays
set(EMBED_SCRIPT_PATH ${CMAKE_BINARY_DIR}/embed_headers.py)
file(WRITE ${EMBED_SCRIPT_PATH}
"#!/usr/bin/env python3
import os
import sys

def file_to_hex(path):
    try:
        with open(path, 'rb') as f:
            content = f.read()
        return ', '.join([f'0x{b:02x}' for b in content])
    except:
        return '0x00'

def main():
    if len(sys.argv) != 3:
        print('Usage: embed_headers.py <source_dir> <output_file>')
        sys.exit(1)
    
    source_dir = sys.argv[1] 
    output_file = sys.argv[2]
    
    # Essential headers only for embedded system
    essential_headers = [
        'stdio.h', 'stdlib.h', 'string.h', 'stddef.h', 'stdint.h',
        'unistd.h', 'fcntl.h', 'sys/types.h', 'sys/stat.h'
    ]
    
    with open(output_file, 'w') as out:
        out.write('#include \"../runtime/header_extractor.h\"\\\\n\\\\n')
        out.write('// Auto-generated embedded headers\\\\n')
        out.write('// Tesla Consciousness Computing - Header Bundle\\\\n\\\\n')
        
        headers = []
        for header in essential_headers:
            header_path = os.path.join(source_dir, header)
            if os.path.exists(header_path):
                headers.append((header, header_path))
        
        # Generate embedded data
        for rel_path, full_path in headers:
            var_name = rel_path.replace('/', '_').replace('.', '_').replace('-', '_')
            hex_data = file_to_hex(full_path)
            
            out.write(f'static const unsigned char {var_name}_data[] = {{{hex_data}}};\\\\n')
        
        # Generate header table
        out.write('\\\\nconst EmbeddedHeader embedded_headers[] = {\\\\n')
        for rel_path, _ in headers:
            var_name = rel_path.replace('/', '_').replace('.', '_').replace('-', '_')
            out.write(f'    {{\\\"{rel_path}\\\", {var_name}_data, sizeof({var_name}_data)}},\\\\n')
        out.write('};\\\\n\\\\n')
        
        out.write(f'const size_t embedded_headers_count = {len(headers)};\\\\n')
        out.write('#define EMBEDDED_HEADERS_AVAILABLE\\\\n')

if __name__ == '__main__':
    main()
")

# Standard C headers to bundle
set(STANDARD_HEADERS_DIR "/usr/include")
set(EMBEDDED_HEADERS_OUTPUT "${CMAKE_BINARY_DIR}/embedded_headers.cpp")

# Custom target to bundle headers
add_custom_target(bundle_headers
    COMMAND python3 ${EMBED_SCRIPT_PATH} ${STANDARD_HEADERS_DIR} ${EMBEDDED_HEADERS_OUTPUT}
    COMMENT "Bundling standard C headers..."
    VERBATIM
)

# Function to add header bundling to targets
function(target_bundle_headers target_name)
    add_dependencies(${target_name} bundle_headers)
    target_sources(${target_name} PRIVATE ${EMBEDDED_HEADERS_OUTPUT})
endfunction()

message(STATUS "✅ Header bundling system ready")