#pragma once

#include <string>
#include <vector>
#include <unordered_map>

// Forward declaration of embedded header data
struct EmbeddedHeader {
    const char* name;
    const unsigned char* data;
    size_t size;
};

class HeaderExtractor {
public:
    HeaderExtractor() = default;
    ~HeaderExtractor();
    
    // Extract all standard headers to temporary directory
    std::string extract_standard_headers();
    
    // Extract specific header
    bool extract_header(const std::string& header_name, const std::string& output_path);
    
    // Check if header is available
    bool is_header_available(const std::string& header_name);
    
    // Get list of all embedded headers
    std::vector<std::string> get_available_headers();
    
    // Cleanup extracted headers
    void cleanup();
    
private:
    std::string extraction_path;
    std::unordered_map<std::string, const EmbeddedHeader*> header_map;
    
    // Initialize header mapping
    void initialize_header_map();
    
    // Write binary data to file
    bool write_header_file(const std::string& file_path, 
                          const unsigned char* data, 
                          size_t size);
    
    // Create directory structure for headers
    bool create_header_directory_structure(const std::string& base_path);
};