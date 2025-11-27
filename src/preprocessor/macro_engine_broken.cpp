#include "macro_engine.h"
#include <regex>
#include <sstream>
#include <iostream>

bool MacroCondition::evaluate(const std::unordered_map<std::string, std::string>& variables) const {
    // Get actual values, supporting both variables and literals
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
    
    double left_val = get_value(left_operand);
    double right_val = get_value(right_operand);
    
    switch (operation) {
        case ConditionalOp::EQUAL:      return left_val == right_val;
        case ConditionalOp::NOT_EQUAL:  return left_val != right_val;
        case ConditionalOp::GREATER:    return left_val > right_val;
        case ConditionalOp::LESS:       return left_val < right_val;
        case ConditionalOp::GREATER_EQ: return left_val >= right_val;
        case ConditionalOp::LESS_EQ:    return left_val <= right_val;
        default: return false;
    }
}

std::string MacroEngine::expand_macros(const std::string& source_code,
                                      const std::unordered_map<std::string, std::string>& vars) {
    // Merge with current variables
    auto merged_vars = variables;
    for (const auto& [key, value] : vars) {
        merged_vars[key] = value;
    }
    
    std::string result = source_code;
    
    // Process Tesla conditional macros first
    result = process_tesla_conditionals(result);
    
    // Process meta-repeat constructs
    result = process_meta_repeat(result);
    
    // Process consciousness macros
    result = process_consciousness_macros(result);
    
    // Final variable substitution
    result = substitute_variables(result);
    
    return result;
}

std::string MacroEngine::process_tesla_conditionals(const std::string& code) {
    std::string result = code;
    
    // Pattern for your exact syntax: @tesla_if(var, op, value) { ... } @tesla_else { ... }
    std::regex tesla_if_pattern(R"(@tesla_if\s*\(\s*(\w+)\s*([!=<>]+)\s*(\w+)\s*\)\s*\{([^}]*)\}(?:\s*@tesla_else\s*\{([^}]*)\})?)");
    
    std::smatch match;
    while (std::regex_search(result, match, tesla_if_pattern)) {
        std::string var_name = match[1].str();
        std::string operator_str = match[2].str();
        std::string compare_value = match[3].str();
        std::string true_body = match[4].str();
        std::string false_body = match.size() > 5 ? match[5].str() : "";
        
        // Parse condition
        MacroCondition condition;
        condition.left_operand = var_name;
        condition.operation = parse_operator(operator_str);
        condition.right_operand = compare_value;
        
        // Evaluate condition
        bool condition_result = condition.evaluate(variables);
        
        // Choose appropriate body
        std::string replacement_body = condition_result ? true_body : false_body;
        
        // Add Tesla consciousness sync if needed
        if (replacement_body.find("tesla_sync") == std::string::npos) {
            replacement_body = "\n    // Tesla consciousness sync\n    tesla_sync();\n" + replacement_body;
        }
        
        // Replace the entire macro with the chosen body
        result.replace(match.position(), match.length(), replacement_body);
        expansion_count++;
    }
    
    return result;
}\n\nstd::string MacroEngine::process_meta_repeat(const std::string& code) {\n    std::string result = code;\n    \n    // Pattern for: @tesla_repeat(count, var) { ... }\n    std::regex repeat_pattern(R\"(@tesla_repeat\\s*\\(\\s*(\\d+)\\s*,\\s*(\\w+)\\s*\\)\\s*\\{([^}]*)\\})\");\n    \n    std::smatch match;\n    while (std::regex_search(result, match, repeat_pattern)) {\n        int count = std::stoi(match[1].str());\n        std::string var_name = match[2].str();\n        std::string body = match[3].str();\n        \n        std::stringstream expanded;\n        expanded << \"\\n    // Tesla meta-repeat expansion (\" << count << \" iterations)\\n\";\n        \n        for (int i = 0; i < count; i++) {\n            // Set iteration variable\n            set_variable(var_name, i);\n            \n            // Expand body with current iteration\n            std::string iteration_body = substitute_variables(body);\n            expanded << \"    // Iteration \" << i << \"\\n\";\n            expanded << iteration_body << \"\\n\";\n        }\n        \n        result.replace(match.position(), match.length(), expanded.str());\n        expansion_count++;\n    }\n    \n    return result;\n}\n\nstd::string MacroEngine::process_consciousness_macros(const std::string& code) {\n    std::string result = code;\n    \n    // Replace consciousness directives with actual code\n    std::regex consciousness_pattern(R\"(@tesla_consciousness_sync\\s*\\(\\s*\\))\");\n    std::string sync_code = R\"(\n    // Tesla consciousness synchronization at π Hz\n    mov rax, 3141592653589793  ; π * 10^15\n    mov rbx, 1000000000000000  ; 10^15  \n    ; Consciousness sync complete\n    )\";\n    \n    result = std::regex_replace(result, consciousness_pattern, sync_code);\n    \n    return result;\n}\n\nConditionalOp MacroEngine::parse_operator(const std::string& op_text) {\n    if (op_text == \"==\" || op_text == \"eq\") return ConditionalOp::EQUAL;\n    if (op_text == \"!=\" || op_text == \"ne\") return ConditionalOp::NOT_EQUAL;\n    if (op_text == \">\" || op_text == \"gt\") return ConditionalOp::GREATER;\n    if (op_text == \"<\" || op_text == \"lt\") return ConditionalOp::LESS;\n    if (op_text == \">=\" || op_text == \"ge\") return ConditionalOp::GREATER_EQ;\n    if (op_text == \"<=\" || op_text == \"le\") return ConditionalOp::LESS_EQ;\n    return ConditionalOp::UNKNOWN;\n}\n\nvoid MacroEngine::set_variable(const std::string& name, const std::string& value) {\n    variables[name] = value;\n}\n\nvoid MacroEngine::set_variable(const std::string& name, int value) {\n    variables[name] = std::to_string(value);\n}\n\nvoid MacroEngine::set_variable(const std::string& name, double value) {\n    variables[name] = std::to_string(value);\n}\n\nstd::string MacroEngine::get_variable(const std::string& name) const {\n    auto it = variables.find(name);\n    return (it != variables.end()) ? it->second : \"\";\n}\n\nstd::string MacroEngine::substitute_variables(const std::string& text) {\n    std::string result = text;\n    \n    // Replace variable references like ${var} or %[var]\n    for (const auto& [name, value] : variables) {\n        std::string var_pattern = \"\\\\${\" + name + \"}\";\n        std::string nasm_pattern = \"%\\\\[\" + name + \"\\\\]\";\n        \n        result = std::regex_replace(result, std::regex(var_pattern), value);\n        result = std::regex_replace(result, std::regex(nasm_pattern), value);\n    }\n    \n    return result;\n}"