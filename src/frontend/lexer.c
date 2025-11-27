/* Aria_lang/src/frontend/lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

static const char* start;
static const char* current;
static int line;

void init_lexer(const char* source) {
    start = source;
    current = source;
    line = 1;
}

static int is_at_end() {
    return *current == '\0';
}

static char advance() {
    current++;
    return current[-1];
}

static char peek() {
    return *current;
}

static char peek_next() {
    if (is_at_end()) return '\0';
    return current[1];
}

static int match_char(char expected) {
    if (is_at_end()) return 0;
    if (*current != expected) return 0;
    current++;
    return 1;
}

static Token make_token(TokenType type) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = (int)(current - start);
    token.line = line;
    return token;
}

static Token error_token(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = line;
    return token;
}

static void skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
                advance();
                break;
            case '\r':
                advance();
                if (peek() == '\n') {
                    advance();
                    line++;
                }
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_at_end()) advance();
                } 
                else if (peek_next() == '*') {
                    advance();
                    advance();
                    while (!is_at_end()) {
                        if (peek() == '*' && peek_next() == '/') {
                            advance();
                            advance();
                            break;
                        }
                        if (peek() == '\n') line++;
                        advance();
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType check_keyword(int start_len, int length, const char* rest, TokenType type) {
    if (current - start == start_len + length && memcmp(start + start_len, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type() {
    switch (*start) {
        case 'b': return check_keyword(1, 4, "reak", TOKEN_BREAK);
        case 'c': 
            if (current - start > 1) {
                switch(start[1]) {
                    case 'l': return check_keyword(2, 3, "ass", TOKEN_CLASS);
                    case 'o': return check_keyword(2, 6, "ntinue", TOKEN_CONTINUE);
                }
            }
            break;
        case 'e': return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'f': 
            if (current - start > 1) {
                switch(start[1]) {
                    case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return check_keyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return check_keyword(2, 2, "nc", TOKEN_FUNC);
                }
            }
            break;
        case 'i': 
            if (current - start == 2) {
                if (start[1] == 'f') return TOKEN_IF;
                if (start[1] == 's') return TOKEN_IS;
            }
            return TOKEN_IDENTIFIER;
        case 'm': return check_keyword(1, 6, "anaged", TOKEN_MANAGED);
        case 'n': 
             if (current - start > 1) {
                 switch(start[1]) {
                     case 'e': return check_keyword(2, 1, "w", TOKEN_NEW);
                     case 'u': return check_keyword(2, 2, "ll", TOKEN_NULL);
                 }
             }
             break;
        case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 't': return check_keyword(1, 3, "rue", TOKEN_TRUE);
        case 'v': return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // FIX: Corrected OR syntax
    while (isalnum(peek()) || peek() == '_') advance();
    return make_token(identifier_type());
}

static Token number() {
    // Hex Literals (0x...)
    // FIX: Corrected OR syntax
    if (peek() == '0' && (peek_next() == 'x' || peek_next() == 'X')) {
        advance(); advance(); 
        while (isxdigit(peek())) advance();
        return make_token(TOKEN_NUMBER);
    }
    // Binary Literals (0b...)
    // FIX: Corrected OR syntax
    if (peek() == '0' && (peek_next() == 'b' || peek_next() == 'B')) {
        advance(); advance(); 
        // FIX: Corrected OR syntax
        while (peek() == '0' || peek() == '1') advance();
        return make_token(TOKEN_NUMBER);
    }

    while (isdigit(peek())) advance();

    // Floating Point detection
    if (peek() == '.' && isdigit(peek_next())) {
        advance(); 
        while (isdigit(peek())) advance();
        return make_token(TOKEN_FLOAT);
    }

    return make_token(TOKEN_NUMBER);
}

static Token string_val() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') line++;
        if (peek() == '\\') {
            advance(); 
            if (!is_at_end()) advance(); 
        } else {
            advance();
        }
    }
    if (is_at_end()) return error_token("Unterminated string.");
    advance(); 
    return make_token(TOKEN_STRING);
}

Token scan_token() {
    skip_whitespace();
    start = current;
    
    if (is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();
    
    // FIX: Corrected OR syntax
    if (isalpha(c) || c == '_') return identifier();
    if (isdigit(c)) return number();

    switch (c) {
        case '(': return make_token(TOKEN_LPAREN);
        case ')': return make_token(TOKEN_RPAREN);
        case '{': return make_token(TOKEN_LBRACE);
        case '}': return make_token(TOKEN_RBRACE);
        case '[': return make_token(TOKEN_LBRACKET);
        case ']': return make_token(TOKEN_RBRACKET); 
        case ';': return make_token(TOKEN_SEMICOLON);
        case ',': return make_token(TOKEN_COMMA);
        case '.': return make_token(TOKEN_DOT);
        case '?': return make_token(TOKEN_QUESTION);
        case ':': return make_token(TOKEN_COLON);
        
        case '-': 
            if (match_char('>')) return make_token(TOKEN_ARROW);
            if (match_char('=')) return make_token(TOKEN_MINUS_EQ); 
            return make_token(TOKEN_MINUS);
        case '+': 
            if (match_char('=')) return make_token(TOKEN_PLUS_EQ); 
            return make_token(TOKEN_PLUS);
        case '*': 
            if (match_char('=')) return make_token(TOKEN_STAR_EQ); 
            return make_token(TOKEN_STAR);
        case '/': 
            if (match_char('=')) return make_token(TOKEN_SLASH_EQ); 
            return make_token(TOKEN_SLASH);
        case '%': return make_token(TOKEN_PERCENT);
        case '!': return make_token(match_char('=') ? TOKEN_NEQ : TOKEN_BANG); 
        case '=': return make_token(match_char('=') ? TOKEN_EQEQ : TOKEN_EQ);
        case '<': return make_token(match_char('=') ? TOKEN_LTEQ : TOKEN_LT);
        case '>': return make_token(match_char('=') ? TOKEN_GTEQ : TOKEN_GT);
        case '&': if (match_char('&')) return make_token(TOKEN_AND); break;
        case '|': if (match_char('|')) return make_token(TOKEN_OR); break;
        case '"': return string_val();
    }

    return error_token("Unexpected character.");
}
