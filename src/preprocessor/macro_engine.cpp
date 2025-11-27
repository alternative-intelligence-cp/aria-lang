#include "macro_engine.h"
#include <regex>
#include <sstream>
#include <iostream>
#include <memory>
#include <cmath>

// MacroCondition constructors
MacroCondition::MacroCondition(const std::string& left, ConditionalOp op, const std::string& right)
    : left_operand(left), operation(op), right_operand(right), is_compound(false) {}

MacroCondition::MacroCondition(std::unique_ptr<MacroCondition> left, ConditionalOp op, std::unique_ptr<MacroCondition> right)
    : left_condition(std::move(left)), operation(op), right_condition(std::move(right)), is_compound(true) {}

bool MacroCondition::evaluate(const std::unordered_map<std::string, std::string>& variables) const {
    if (is_compound) {
        // Handle compound conditions
        if (!left_condition || !right_condition) return false;
        
        bool left_result = left_condition->evaluate(variables);
        bool right_result = right_condition->evaluate(variables);
        
        switch (operation) {
            case ConditionalOp::LOGICAL_AND: return left_result && right_result;
            case ConditionalOp::LOGICAL_OR:  return left_result || right_result;
            default: return false;
        }
    }
    
    // Handle simple conditions
    auto get_value = [&](const std::string& operand) -> double {
        if (variables.find(operand) != variables.end()) {
            try {
                return std::stod(variables.at(operand));
            } catch (...) {
                return 0.0;
            }
        }
        
        try {
            return std::stod(operand);
        } catch (...) {
            return 0.0;
        }
    };
    
    auto get_string = [&](const std::string& operand) -> std::string {
        if (variables.find(operand) != variables.end()) {
            return variables.at(operand);
        }
        return operand;
    };
    
    auto is_defined = [&](const std::string& operand) -> bool {
        return variables.find(operand) != variables.end();
    };
    
    // Handle unary operators first
    if (operation == ConditionalOp::LOGICAL_NOT) {
        double val = get_value(left_operand);
        return val == 0.0; // Tesla style: 0 is false, anything else is true
    }
    
    if (operation == ConditionalOp::IS_DEFINED) {
        return is_defined(left_operand);
    }
    
    if (operation == ConditionalOp::IS_EMPTY) {
        std::string str = get_string(left_operand);
        return str.empty() || str == "0" || str == "null";
    }
    
    // Handle binary numeric operators
    double val1 = get_value(left_operand);
    double val2 = get_value(right_operand);
    
    switch (operation) {
        case ConditionalOp::EQUAL:       return val1 == val2;
        case ConditionalOp::NOT_EQUAL:   return val1 != val2;
        case ConditionalOp::GREATER:     return val1 > val2;
        case ConditionalOp::LESS:        return val1 < val2;
        case ConditionalOp::GREATER_EQ:  return val1 >= val2;
        case ConditionalOp::LESS_EQ:     return val1 <= val2;
        case ConditionalOp::MODULO:      return val2 != 0.0 && (fmod(val1, val2) == 0.0);
        case ConditionalOp::DIVISIBLE:   return val2 != 0.0 && (fmod(val1, val2) == 0.0);
        
        // String operations
        case ConditionalOp::CONTAINS: {
            std::string str1 = get_string(left_operand);
            std::string str2 = get_string(right_operand);
            return str1.find(str2) != std::string::npos;
        }
        case ConditionalOp::STARTS_WITH: {
            std::string str1 = get_string(left_operand);
            std::string str2 = get_string(right_operand);
            return str1.substr(0, str2.length()) == str2;
        }
        case ConditionalOp::ENDS_WITH: {
            std::string str1 = get_string(left_operand);
            std::string str2 = get_string(right_operand);
            if (str2.length() > str1.length()) return false;
            return str1.substr(str1.length() - str2.length()) == str2;
        }
        
        default: return false;
    }
}

std::string MacroEngine::expand_macros(const std::string& source_code,
                                     const std::unordered_map<std::string, std::string>& vars) {
    // Set provided variables
    for (const auto& [name, value] : vars) {
        variables[name] = value;
    }
    
    std::string result = source_code;
    
    std::cout << "🔧 Processing Tesla macros...\n";
    
    // Process in order: conditionals → meta-repeat → consciousness
    result = process_tesla_conditionals(result);
    result = process_meta_repeat(result);
    result = process_consciousness_macros(result);
    
    std::cout << "✅ Macro processing complete (" << expansion_count << " expansions)\n";
    return result;
}

std::string MacroEngine::process_tesla_conditionals(const std::string& code) {
    std::string result = code;
    
    // Enhanced pattern for compound conditions: @tesla_if(condition)
    // Supports: @tesla_if(x > 5 && y < 10) or @tesla_if(a == 1 || b != 0)
    std::regex if_pattern(R"(@tesla_if\s*\(\s*([^)]+)\s*\))");
    
    std::smatch match;
    while (std::regex_search(result, match, if_pattern)) {
        std::string condition_text = match[1].str();
        
        // Parse the condition (handles both simple and compound)
        auto condition = parse_compound_condition(condition_text);
        if (!condition) {
            std::cout << "⚠️ Invalid condition syntax: " << condition_text << "\n";
            break;
        }
        
        // Evaluate condition
        bool condition_result = condition->evaluate(variables);
        
        std::cout << "🔍 Tesla conditional: " << condition_text 
                  << " → " << (condition_result ? "true" : "false") << "\n";
        
        // Replace with result comment for now
        std::string replacement = "\n    ; Tesla conditional: " + condition_text + 
                                " = " + (condition_result ? "true" : "false") + "\n";
        
        result.replace(match.position(), match.length(), replacement);
        expansion_count++;
    }
    
    return result;
}

std::string MacroEngine::process_meta_repeat(const std::string& code) {
    std::string result = code;
    
    // Pattern for: @tesla_repeat(count, var) { ... }
    // Use a simpler approach to avoid regex issues
    size_t pos = 0;
    while ((pos = result.find("@tesla_repeat", pos)) != std::string::npos) {
        // Find the opening parenthesis
        size_t paren_start = result.find('(', pos);
        if (paren_start == std::string::npos) break;
        
        // Find the closing parenthesis
        size_t paren_end = result.find(')', paren_start);
        if (paren_end == std::string::npos) break;
        
        // Extract parameters
        std::string params = result.substr(paren_start + 1, paren_end - paren_start - 1);
        size_t comma_pos = params.find(',');
        if (comma_pos == std::string::npos) {
            pos = paren_end + 1;
            continue;
        }
        
        std::string count_str = params.substr(0, comma_pos);
        std::string var_name = params.substr(comma_pos + 1);
        
        // Trim whitespace
        count_str.erase(0, count_str.find_first_not_of(" \t"));
        count_str.erase(count_str.find_last_not_of(" \t") + 1);
        var_name.erase(0, var_name.find_first_not_of(" \t"));
        var_name.erase(var_name.find_last_not_of(" \t") + 1);
        
        // Find the opening brace
        size_t brace_start = result.find('{', paren_end);
        if (brace_start == std::string::npos) {
            pos = paren_end + 1;
            continue;
        }
        
        // Find the matching closing brace
        size_t brace_end = brace_start + 1;
        int brace_count = 1;
        while (brace_end < result.length() && brace_count > 0) {
            if (result[brace_end] == '{') brace_count++;
            else if (result[brace_end] == '}') brace_count--;
            brace_end++;
        }
        
        if (brace_count > 0) {
            pos = brace_start + 1;
            continue; // No matching closing brace
        }
        
        brace_end--; // Point to the actual closing brace
        
        std::string body = result.substr(brace_start + 1, brace_end - brace_start - 1);
        
        try {
            int count = std::stoi(count_str);
            
            std::stringstream expanded;
            expanded << "\n    ; Tesla meta-repeat expansion (" << count << " iterations)\n";
            
            for (int i = 0; i < count; i++) {
                // Set iteration variable
                set_variable(var_name, i);
                
                // Expand body with current iteration
                std::string iteration_body = substitute_variables(body);
                expanded << "    ; Iteration " << i << "\n";
                expanded << iteration_body << "\n";
            }
            
            // Replace the entire macro with expanded code
            result.replace(pos, brace_end + 1 - pos, expanded.str());
            expansion_count++;
        } catch (...) {
            pos = brace_end + 1;
        }
    }
    
    return result;
}

std::string MacroEngine::process_consciousness_macros(const std::string& code) {
    std::string result = code;
    
    // Replace consciousness directives with actual code
    std::regex consciousness_pattern(R"(@tesla_consciousness_sync\s*\(\s*\))");
    std::string sync_code = R"(
    ; Tesla consciousness synchronization at π Hz
    mov rax, 3141592653589793  ; π * 10^15
    mov rbx, 1000000000000000  ; 10^15  
    ; Consciousness sync complete
    )";
    
    result = std::regex_replace(result, consciousness_pattern, sync_code);
    
    return result;
}

ConditionalOp MacroEngine::parse_operator(const std::string& op_text) {
    // Comparison operators (multiple aliases for ease of use)
    if (op_text == "==" || op_text == "eq") return ConditionalOp::EQUAL;
    if (op_text == "!=" || op_text == "ne" || op_text == "not" || op_text == "<>") return ConditionalOp::NOT_EQUAL;
    if (op_text == ">" || op_text == "gt") return ConditionalOp::GREATER;
    if (op_text == "<" || op_text == "lt") return ConditionalOp::LESS;
    if (op_text == ">=" || op_text == "ge") return ConditionalOp::GREATER_EQ;
    if (op_text == "<=" || op_text == "le") return ConditionalOp::LESS_EQ;
    
    // Logical operators
    if (op_text == "&&" || op_text == "and") return ConditionalOp::LOGICAL_AND;
    if (op_text == "||" || op_text == "or") return ConditionalOp::LOGICAL_OR;
    if (op_text == "!" || op_text == "not") return ConditionalOp::LOGICAL_NOT;
    
    // String operators
    if (op_text == "contains" || op_text == "in") return ConditionalOp::CONTAINS;
    if (op_text == "starts_with" || op_text == "begins") return ConditionalOp::STARTS_WITH;
    if (op_text == "ends_with" || op_text == "suffix") return ConditionalOp::ENDS_WITH;
    
    // Numeric operators
    if (op_text == "%" || op_text == "mod") return ConditionalOp::MODULO;
    if (op_text == "divisible_by" || op_text == "div") return ConditionalOp::DIVISIBLE;
    
    // Type checking
    if (op_text == "defined" || op_text == "exists") return ConditionalOp::IS_DEFINED;
    if (op_text == "empty" || op_text == "null") return ConditionalOp::IS_EMPTY;
    
    return ConditionalOp::UNKNOWN;
}

void MacroEngine::set_variable(const std::string& name, const std::string& value) {
    variables[name] = value;
}

void MacroEngine::set_variable(const std::string& name, int value) {
    variables[name] = std::to_string(value);
}

void MacroEngine::set_variable(const std::string& name, double value) {
    variables[name] = std::to_string(value);
}

std::string MacroEngine::get_variable(const std::string& name) const {
    auto it = variables.find(name);
    return (it != variables.end()) ? it->second : "";
}

std::string MacroEngine::substitute_variables(const std::string& text) {
    std::string result = text;
    
    // Replace variable references like ${var} or %[var] using simple string replacement
    for (const auto& [name, value] : variables) {
        std::string var_pattern = "${" + name + "}";
        std::string nasm_pattern = "%[" + name + "]";
        
        // Simple string replacement to avoid regex issues
        size_t pos = 0;
        while ((pos = result.find(var_pattern, pos)) != std::string::npos) {
            result.replace(pos, var_pattern.length(), value);
            pos += value.length();
        }
        
        pos = 0;
        while ((pos = result.find(nasm_pattern, pos)) != std::string::npos) {
            result.replace(pos, nasm_pattern.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

bool MacroEngine::register_conditional_macro(const std::string& macro_text) {
    // Implementation for registering conditional macros
    // For now, return true as placeholder
    return true;
}

std::unique_ptr<MacroCondition> MacroEngine::parse_compound_condition(const std::string& condition_text) {
    // First check for compound operators (&& or ||)
    size_t and_pos = condition_text.find("&&");
    size_t or_pos = condition_text.find("||");
    
    if (and_pos != std::string::npos || or_pos != std::string::npos) {
        // Determine which operator comes first
        size_t split_pos;
        ConditionalOp compound_op;
        
        if (and_pos != std::string::npos && (or_pos == std::string::npos || and_pos < or_pos)) {
            split_pos = and_pos;
            compound_op = ConditionalOp::LOGICAL_AND;
        } else {
            split_pos = or_pos;
            compound_op = ConditionalOp::LOGICAL_OR;
        }
        
        // Split into left and right parts
        std::string left_part = condition_text.substr(0, split_pos);
        std::string right_part = condition_text.substr(split_pos + 2);
        
        // Trim whitespace
        left_part.erase(0, left_part.find_first_not_of(" \t"));
        left_part.erase(left_part.find_last_not_of(" \t") + 1);
        right_part.erase(0, right_part.find_first_not_of(" \t"));
        right_part.erase(right_part.find_last_not_of(" \t") + 1);
        
        // Recursively parse both sides
        auto left_condition = parse_compound_condition(left_part);
        auto right_condition = parse_compound_condition(right_part);
        
        if (!left_condition || !right_condition) {
            return nullptr;
        }
        
        return std::make_unique<MacroCondition>(std::move(left_condition), compound_op, std::move(right_condition));
    }
    
    // Parse simple condition using a more robust approach
    std::string trimmed = condition_text;
    // Remove leading/trailing whitespace
    trimmed.erase(0, trimmed.find_first_not_of(" \\t"));
    trimmed.erase(trimmed.find_last_not_of(" \\t") + 1);
    
    // Check for unary operators first (defined, empty, etc.)
    std::vector<std::string> unary_ops = {"defined", "exists", "empty", "null"};
    for (const auto& op : unary_ops) {
        if (trimmed.find(op) != std::string::npos) {
            // For unary operators, the variable name comes before the operator
            size_t op_pos = trimmed.find(op);
            if (op_pos > 0) {
                std::string operand = trimmed.substr(0, op_pos);
                operand.erase(0, operand.find_first_not_of(" \\t"));
                operand.erase(operand.find_last_not_of(" \\t") + 1);
                
                if (!operand.empty()) {
                    ConditionalOp parsed_op = parse_operator(op);
                    return std::make_unique<MacroCondition>(operand, parsed_op, "");
                }
            }
        }
    }
    
    // Parse binary conditions
    std::vector<std::string> operators = {
        // Multi-character operators first
        "divisible_by", "starts_with", "ends_with", "contains", 
        "==", "!=", ">=", "<=", "<>", "&&", "||",
        "eq", "ne", "gt", "lt", "ge", "le", "not", "and", "or", "mod", "div", "in",
        "begins", "suffix",
        // Single character operators last  
        ">", "<", "=", "!", "%"
    };
    
    for (const auto& op : operators) {
        size_t pos = trimmed.find(op);
        if (pos != std::string::npos) {
            std::string operand1 = trimmed.substr(0, pos);
            std::string operand2 = trimmed.substr(pos + op.length());
            
            // Trim operands
            operand1.erase(0, operand1.find_first_not_of(" \\t"));
            operand1.erase(operand1.find_last_not_of(" \\t") + 1);
            operand2.erase(0, operand2.find_first_not_of(" \\t"));
            operand2.erase(operand2.find_last_not_of(" \\t") + 1);
            
            if (!operand1.empty()) {  // operand2 can be empty for unary ops
                ConditionalOp parsed_op = parse_operator(op);
                if (parsed_op != ConditionalOp::UNKNOWN) {
                    return std::make_unique<MacroCondition>(operand1, parsed_op, operand2);
                }
            }
        }
    }
    
    return nullptr;
}

std::string MacroEngine::extract_macro_body(const std::string& source, size_t start_pos, 
                                          const std::string& end_marker) {
    // Find end marker and extract body
    size_t end_pos = source.find(end_marker, start_pos);
    if (end_pos != std::string::npos) {
        return source.substr(start_pos, end_pos - start_pos);
    }
    return "";
}

bool MacroEngine::evaluate_condition(const MacroCondition& condition) {
    return condition.evaluate(variables);
}

void MacroEngine::inject_consciousness_sync_points(std::string& code) {
    // Add consciousness synchronization points at key locations
    std::string sync_point = "\n    ; Tesla consciousness sync point\n    nop\n";
    
    // Add sync points before function calls, loops, etc.
    code = std::regex_replace(code, std::regex(R"(call\s+\w+)"), sync_point + "$&");
}