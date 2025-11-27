/* Aria_lang/src/frontend/token.h */
#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Keywords
    TOKEN_FUNC, TOKEN_VAR, TOKEN_RETURN, 
    TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, 
    TOKEN_CLASS, TOKEN_MANAGED, TOKEN_NEW,
    TOKEN_TRUE, TOKEN_FALSE, TOKEN_NULL,
    TOKEN_BREAK, TOKEN_CONTINUE,
    
    // Feature: Ternary Operator
    // The user requested 'is' for the ternary condition.
    TOKEN_IS, 

    // Literals
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_FLOAT, TOKEN_STRING,

    // Delimiters
    TOKEN_LPAREN, TOKEN_RPAREN, 
    TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_LBRACKET, TOKEN_RBRACKET, 
    TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT,
    TOKEN_COLON, TOKEN_QUESTION, // Added for ternary syntax support

    // Operators
    TOKEN_PLUS, TOKEN_MINUS, 
    TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT,
    TOKEN_EQ, TOKEN_EQEQ, TOKEN_NEQ,
    TOKEN_LT, TOKEN_LTEQ, 
    TOKEN_GT, TOKEN_GTEQ,
    TOKEN_AND, TOKEN_OR, TOKEN_BANG, 
    TOKEN_ARROW,

    // Compound Assignments
    TOKEN_PLUS_EQ, TOKEN_MINUS_EQ,
    TOKEN_STAR_EQ, TOKEN_SLASH_EQ,

    // Error and State Tokens
    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

Token scan_token();

#endif
