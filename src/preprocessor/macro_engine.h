#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

// Conditional operators for macro system
enum class ConditionalOp {
    // Comparison operators
    EQUAL,      // eq, ==
    NOT_EQUAL,  // ne, !=, not, <>
    GREATER,    // gt, >
    LESS,       // lt, <
    GREATER_EQ, // ge, >=
    LESS_EQ,    // le, <=
    
    // Logical operators
    LOGICAL_AND, // &&, and
    LOGICAL_OR,  // ||, or
    LOGICAL_NOT, // !, not
    
    // String operators
    CONTAINS,    // contains, in
    STARTS_WITH, // starts_with, begins
    ENDS_WITH,   // ends_with, suffix
    
    // Numeric operators
    MODULO,      // %, mod
    DIVISIBLE,   // divisible_by, div
    
    // Type checking
    IS_DEFINED,  // defined, exists
    IS_EMPTY,    // empty, null
    
    UNKNOWN
};

struct MacroCondition {
    std::string left_operand;
    ConditionalOp operation;
    std::string right_operand;
    
    // Compound condition support
    std::unique_ptr<MacroCondition> left_condition;
    std::unique_ptr<MacroCondition> right_condition;
    bool is_compound = false;
    
    // Constructors
    MacroCondition() = default;
    MacroCondition(const std::string& left, ConditionalOp op, const std::string& right);
    MacroCondition(std::unique_ptr<MacroCondition> left, ConditionalOp op, std::unique_ptr<MacroCondition> right);
    
    // Copy/move for unique_ptr
    MacroCondition(const MacroCondition&) = delete;
    MacroCondition& operator=(const MacroCondition&) = delete;
    MacroCondition(MacroCondition&&) = default;
    MacroCondition& operator=(MacroCondition&&) = default;
    
    bool evaluate(const std::unordered_map<std::string, std::string>& variables) const;
};

struct ConditionalMacro {
    std::string name;
    MacroCondition condition;
    std::string true_body;
    std::string false_body;
    bool has_else = false;
    
    // For complex if-elseif-else chains
    std::vector<std::pair<MacroCondition, std::string>> elseif_branches;
};

class MacroEngine {
public:
    MacroEngine() = default;
    
    // Register your exact conditional syntax: @tesla_if(v1, conditional, v2)
    bool register_conditional_macro(const std::string& macro_text);
    
    // Process all macros in source code
    std::string expand_macros(const std::string& source_code,
                             const std::unordered_map<std::string, std::string>& variables = {});
    
    // Tesla-specific macro processing
    std::string process_tesla_conditionals(const std::string& code);
    std::string process_meta_repeat(const std::string& code);
    std::string process_consciousness_macros(const std::string& code);
    
    // Variable management
    void set_variable(const std::string& name, const std::string& value);
    void set_variable(const std::string& name, int value);
    void set_variable(const std::string& name, double value);
    std::string get_variable(const std::string& name) const;
    
    // Statistics
    int get_expansion_count() const { return expansion_count; }
    void reset_stats() { expansion_count = 0; }
    
private:
    std::unordered_map<std::string, ConditionalMacro> conditional_macros;
    std::unordered_map<std::string, std::string> variables;
    int expansion_count = 0;
    
    // Parsing helpers
    MacroCondition parse_condition(const std::string& condition_text);
    std::unique_ptr<MacroCondition> parse_compound_condition(const std::string& condition_text);
    ConditionalOp parse_operator(const std::string& op_text);
    std::string extract_macro_body(const std::string& source, size_t start_pos, 
                                  const std::string& end_marker);
    
    // Evaluation helpers
    bool evaluate_condition(const MacroCondition& condition);
    std::string substitute_variables(const std::string& text);
    
    // Tesla consciousness integration
    void inject_consciousness_sync_points(std::string& code);
};