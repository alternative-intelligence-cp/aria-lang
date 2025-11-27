// Aria Assembler Detection Engine
// Smart detection system to choose between LLVM-MC and NASM based on code features

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

typedef enum {
    ASSEMBLER_LLVM_MC,    // Fast path - LLVM's built-in assembler
    ASSEMBLER_NASM,       // Power path - Full NASM with macros
    ASSEMBLER_UNKNOWN
} assembler_type_t;

typedef struct {
    bool uses_nasm_macros;
    bool uses_context_stack;
    bool uses_auto_naming;
    bool uses_complex_preprocessing;
    bool uses_local_labels;
    int macro_complexity_score;
} assembly_features_t;

// NASM-specific pattern detection
static const char* nasm_patterns[] = {
    "%macro",           // Macro definitions
    "%endmacro",        // Macro endings
    "%push",            // Context stack push
    "%pop",             // Context stack pop
    "%$",               // Local context labels
    "%%",               // Local labels in macros
    "%define",          // Preprocessor defines
    "%assign",          // Preprocessor assignment
    "%if",              // Conditional assembly
    "%ifdef",           // Conditional ifdef
    "%ifndef",          // Conditional ifndef
    "%else",            // Conditional else
    "%elif",            // Conditional elif
    "%endif",           // Conditional endif
    "%rep",             // Repeat blocks
    "%endrep",          // Repeat endings
    "%include",         // File inclusion
    "%use",             // Standard library usage
    NULL
};

// Auto-naming pattern detection (your specific use case)
static const char* auto_naming_patterns[] = {
    "_wrapper:",        // Generated wrapper functions
    "_instance_",       // Instance-specific functions
    "auto_gen_",        // Auto-generated prefixes
    "%1_move:",         // Parameterized function names
    "%1_update:",       // Parameterized function names
    "%1_draw:",         // Parameterized function names
    NULL
};

// Complexity indicators
static const char* complexity_patterns[] = {
    "%rotate",          // Complex macro operations
    "%substr",          // String manipulation
    "%strlen",          // String length operations
    "..@",              // Anonymous labels
    "$",                // Current location counter
    "$$",               // Section start
    NULL
};

// Check if string contains any of the given patterns
bool contains_patterns(const char* code, const char** patterns) {
    if (!code || !patterns) return false;
    
    for (int i = 0; patterns[i] != NULL; i++) {
        if (strstr(code, patterns[i]) != NULL) {
            return true;
        }
    }
    return false;
}

// Count pattern occurrences for complexity scoring
int count_patterns(const char* code, const char** patterns) {
    if (!code || !patterns) return 0;
    
    int count = 0;
    for (int i = 0; patterns[i] != NULL; i++) {
        const char* pos = code;
        while ((pos = strstr(pos, patterns[i])) != NULL) {
            count++;
            pos += strlen(patterns[i]);
        }
    }
    return count;
}

// Analyze assembly code for NASM-specific features
assembly_features_t analyze_assembly_features(const char* assembly_code) {
    assembly_features_t features = {0};
    
    if (!assembly_code) {
        return features;
    }
    
    // Check for NASM macro usage
    features.uses_nasm_macros = contains_patterns(assembly_code, nasm_patterns);
    
    // Check for context stack usage (push/pop)
    features.uses_context_stack = (strstr(assembly_code, "%push") != NULL) ||
                                  (strstr(assembly_code, "%pop") != NULL);
    
    // Check for auto-naming patterns (your specific use case!)
    features.uses_auto_naming = contains_patterns(assembly_code, auto_naming_patterns);
    
    // Check for complex preprocessing
    features.uses_complex_preprocessing = (strstr(assembly_code, "%if") != NULL) ||
                                         (strstr(assembly_code, "%rep") != NULL) ||
                                         (strstr(assembly_code, "%include") != NULL);
    
    // Check for local labels
    features.uses_local_labels = (strstr(assembly_code, "%%") != NULL) ||
                                (strstr(assembly_code, "%$") != NULL);
    
    // Calculate complexity score
    features.macro_complexity_score = count_patterns(assembly_code, complexity_patterns);
    
    return features;
}

// Smart assembler selection based on detected features
assembler_type_t select_assembler(const assembly_features_t* features) {
    if (!features) {
        return ASSEMBLER_LLVM_MC; // Safe default
    }
    
    // If any NASM-specific features are used, use NASM
    if (features->uses_nasm_macros ||
        features->uses_context_stack ||
        features->uses_auto_naming ||
        features->uses_complex_preprocessing ||
        features->uses_local_labels) {
        return ASSEMBLER_NASM;
    }
    
    // If complexity score is high, prefer NASM
    if (features->macro_complexity_score > 5) {
        return ASSEMBLER_NASM;
    }
    
    // Default to LLVM-MC for speed
    return ASSEMBLER_LLVM_MC;
}

// Pretty print feature analysis
void print_feature_analysis(const assembly_features_t* features, assembler_type_t selected) {
    printf("🔍 Assembly Feature Analysis:\n");
    printf("  NASM Macros: %s\n", features->uses_nasm_macros ? "✅ Detected" : "❌ None");
    printf("  Context Stack: %s\n", features->uses_context_stack ? "✅ Detected" : "❌ None");
    printf("  Auto-Naming: %s\n", features->uses_auto_naming ? "✅ Detected" : "❌ None");
    printf("  Complex Preprocessing: %s\n", features->uses_complex_preprocessing ? "✅ Detected" : "❌ None");
    printf("  Local Labels: %s\n", features->uses_local_labels ? "✅ Detected" : "❌ None");
    printf("  Complexity Score: %d\n", features->macro_complexity_score);
    printf("\n🎯 Selected Assembler: %s\n", 
           selected == ASSEMBLER_NASM ? "🚀 NASM (Advanced Features)" : 
                                       "⚡ LLVM-MC (High Performance)");
}

// Test function for your auto-naming use case
void test_auto_naming_detection() {
    printf("🧪 Testing Auto-Naming Pattern Detection:\n\n");
    
    // Test case 1: Basic LLVM-MC assembly
    const char* basic_asm = 
        "global main\n"
        "section .text\n"
        "main:\n"
        "    mov rax, 0\n"
        "    ret\n";
    
    assembly_features_t basic_features = analyze_assembly_features(basic_asm);
    assembler_type_t basic_assembler = select_assembler(&basic_features);
    
    printf("📝 Test 1 - Basic Assembly:\n");
    print_feature_analysis(&basic_features, basic_assembler);
    printf("\n");
    
    // Test case 2: Your auto-naming pattern
    const char* auto_naming_asm =
        "; Auto-generated player wrappers\n"
        "player1_move:\n"
        "    mov rdi, player1_data\n"
        "    jmp entity_move\n"
        "\n"
        "player2_move:\n"
        "    mov rdi, player2_data\n"
        "    jmp entity_move\n"
        "\n"
        "; NASM macro for auto-generation\n"
        "%macro CREATE_WRAPPER 2\n"
        "%1_wrapper:\n"
        "    mov rdi, %2\n"
        "    jmp %1_impl\n"
        "%endmacro\n";
    
    assembly_features_t auto_features = analyze_assembly_features(auto_naming_asm);
    assembler_type_t auto_assembler = select_assembler(&auto_features);
    
    printf("📝 Test 2 - Auto-Naming with Macros:\n");
    print_feature_analysis(&auto_features, auto_assembler);
    printf("\n");
    
    // Test case 3: Complex NASM with context stack
    const char* complex_asm =
        "%push mycontext\n"
        "%define %$self rdi\n"
        "\n"
        "%macro INSTANCE_METHOD 1\n"
        "%$%1:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    mov %$self, [rbp+16]  ; Get self reference\n"
        "    ; Method implementation here\n"
        "    pop rbp\n"
        "    ret\n"
        "%endmacro\n"
        "\n"
        "INSTANCE_METHOD move\n"
        "INSTANCE_METHOD update\n"
        "INSTANCE_METHOD draw\n"
        "\n"
        "%pop\n";
    
    assembly_features_t complex_features = analyze_assembly_features(complex_asm);
    assembler_type_t complex_assembler = select_assembler(&complex_features);
    
    printf("📝 Test 3 - Complex Context Stack:\n");
    print_feature_analysis(&complex_features, complex_assembler);
}

int main() {
    printf("🎯⚡ ARIA SMART ASSEMBLER DETECTION ENGINE ⚡🎯\n");
    printf("=================================================\n\n");
    
    test_auto_naming_detection();
    
    printf("\n🚀 Detection engine ready for integration!\n");
    printf("💡 Will automatically choose NASM for your auto-naming patterns\n");
    printf("⚡ Will use LLVM-MC for maximum speed on simple assembly\n");
    
    return 0;
}