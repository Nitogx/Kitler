#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
// Main lexer


// The lexer man, why the heck should i name it smth else soso
// Bro, bob4, wtf you meeeaaaaannnnnnnn

// Initialize lexer
Lexer* lexer_init(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start_column = 1;
    return lexer;
}

// Check if we're at the end
static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

// Advance one character
static char advance(Lexer* lexer) {
    char c = *lexer->current;
    lexer->current++;
    lexer->column++;
    return c;
}

// Peek current character
static char peek(Lexer* lexer) {
    return *lexer->current;
}

// Peek next character
static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return *(lexer->current + 1);
}

// Match expected character
static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    advance(lexer);
    return true;
}

// Skip whitespace (except newlines)
static void skip_whitespace(Lexer* lexer) {
    while (true) {
        char c = peek(lexer);
        if (c == ' ' || c == '\t' || c == '\r') {
            advance(lexer);
        } else {
            break;
        }
    }
}

// Skip comments: <-- comment -->
static bool skip_comment(Lexer* lexer) {
    if (peek(lexer) == '<' && peek_next(lexer) == '-') {
        advance(lexer); // <
        advance(lexer); // -
        
        if (peek(lexer) == '-') {
            advance(lexer); // -
            
            // Find closing -->
            while (!is_at_end(lexer)) {
                if (peek(lexer) == '-' && peek_next(lexer) == '-') {
                    advance(lexer);
                    advance(lexer);
                    if (peek(lexer) == '>') {
                        advance(lexer);
                        return true;
                    }
                }
                if (peek(lexer) == '\n') {
                    lexer->line++;
                    lexer->column = 0;
                }
                advance(lexer);
            }
        }
    }
    return false;
}

// Parse string literal
static Token* parse_string(Lexer* lexer) {
    const char* start = lexer->current;
    advance(lexer); // Opening quote
    
    while (!is_at_end(lexer) && peek(lexer) != '"') {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 0;
        }
        advance(lexer);
    }
    
    if (is_at_end(lexer)) {
        Token* token = create_token(TOKEN_ERROR, "Unterminated string", lexer->line, lexer->start_column);
        return token;
    }
    
    int length = lexer->current - start - 1;
    char* string_value = (char*)malloc(length + 1);
    strncpy(string_value, start + 1, length);
    string_value[length] = '\0';
    
    advance(lexer); // Closing quote
    
    Token* token = create_token(TOKEN_STRING, string_value, lexer->line, lexer->start_column);
    token->value.string = string_value;
    return token;
}

// Parse number literal
static Token* parse_number(Lexer* lexer) {
    const char* start = lexer->current;
    
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }
    
    // Check for decimal point
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        advance(lexer); // .
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
    }
    
    int length = lexer->current - start;
    char* number_str = (char*)malloc(length + 1);
    strncpy(number_str, start, length);
    number_str[length] = '\0';
    
    double value = atof(number_str);
    
    Token* token = create_token(TOKEN_NUMBER, number_str, lexer->line, lexer->start_column);
    token->value.number = value;
    
    free(number_str);
    return token;
}

// Parse identifier or keyword
static Token* parse_identifier(Lexer* lexer) {
    const char* start = lexer->current;
    
    while (isalnum(peek(lexer)) || peek(lexer) == '_' || peek(lexer) == '.') {
        advance(lexer);
    }
    
    int length = lexer->current - start;
    char* identifier = (char*)malloc(length + 1);
    strncpy(identifier, start, length);
    identifier[length] = '\0';
    
    // Check for keywords
    TokenType type = TOKEN_IDENTIFIER;
    
    if (strcmp(identifier, "including") == 0) type = TOKEN_INCLUDING;
    else if (strcmp(identifier, "projectSpace") == 0) type = TOKEN_PROJECTSPACE;
    else if (strcmp(identifier, "NewVar") == 0) type = TOKEN_NEWVAR;
    else if (strcmp(identifier, "NewFunc") == 0) type = TOKEN_NEWFUNC;
    else if (strcmp(identifier, "NewClass") == 0) type = TOKEN_NEWCLASS;
    else if (strcmp(identifier, "NewEvent") == 0) type = TOKEN_NEWEVENT;
    else if (strcmp(identifier, "NewAsync") == 0) type = TOKEN_NEWASYNC;
    else if (strcmp(identifier, "if") == 0) type = TOKEN_IF;
    else if (strcmp(identifier, "else") == 0) type = TOKEN_ELSE;
    else if (strcmp(identifier, "while") == 0) type = TOKEN_WHILE;
    else if (strcmp(identifier, "for") == 0) type = TOKEN_FOR;
    else if (strcmp(identifier, "foreach") == 0) type = TOKEN_FOREACH;
    else if (strcmp(identifier, "in") == 0) type = TOKEN_IN;
    else if (strcmp(identifier, "switch") == 0) type = TOKEN_SWITCH;
    else if (strcmp(identifier, "case") == 0) type = TOKEN_CASE;
    else if (strcmp(identifier, "default") == 0) type = TOKEN_DEFAULT;
    else if (strcmp(identifier, "break") == 0) type = TOKEN_BREAK;
    else if (strcmp(identifier, "return") == 0) type = TOKEN_RETURN;
    else if (strcmp(identifier, "run") == 0) type = TOKEN_RUN;
    else if (strcmp(identifier, "end") == 0) type = TOKEN_END;
    else if (strcmp(identifier, "when") == 0) type = TOKEN_WHEN;
    else if (strcmp(identifier, "this") == 0) type = TOKEN_THIS;
    else if (strcmp(identifier, "New") == 0) type = TOKEN_NEW;
    else if (strcmp(identifier, "await") == 0) type = TOKEN_AWAIT;
    else if (strcmp(identifier, "true") == 0) type = TOKEN_TRUE;
    else if (strcmp(identifier, "false") == 0) type = TOKEN_FALSE;
    else if (strcmp(identifier, "and") == 0) type = TOKEN_AND;
    else if (strcmp(identifier, "or") == 0) type = TOKEN_OR;
    
    Token* token = create_token(type, identifier, lexer->line, lexer->start_column);
    return token;
}

// Get next token
Token* lexer_next_token(Lexer* lexer) {
    // Skip whitespace and comments
    while (true) {
        skip_whitespace(lexer);
        if (!skip_comment(lexer)) break;
    }
    
    lexer->start_column = lexer->column;
    
    if (is_at_end(lexer)) {
        return create_token(TOKEN_EOF, "", lexer->line, lexer->column);
    }
    
    char c = peek(lexer);
    
    // Newline
    if (c == '\n') {
        advance(lexer);
        Token* token = create_token(TOKEN_NEWLINE, "\\n", lexer->line, lexer->start_column);
        lexer->line++;
        lexer->column = 1;
        return token;
    }
    
    // String
    if (c == '"') {
        return parse_string(lexer);
    }
    
    // Number
    if (isdigit(c)) {
        return parse_number(lexer);
    }
    
    // Identifier or keyword
    if (isalpha(c) || c == '_') {
        return parse_identifier(lexer);
    }
    
    // Single character tokens
    advance(lexer);
    
    switch (c) {
        case '(': return create_token(TOKEN_LPAREN, "(", lexer->line, lexer->start_column);
        case ')': return create_token(TOKEN_RPAREN, ")", lexer->line, lexer->start_column);
        case '[': return create_token(TOKEN_LBRACKET, "[", lexer->line, lexer->start_column);
        case ']': return create_token(TOKEN_RBRACKET, "]", lexer->line, lexer->start_column);
        case '{': return create_token(TOKEN_LBRACE, "{", lexer->line, lexer->start_column);
        case '}': return create_token(TOKEN_RBRACE, "}", lexer->line, lexer->start_column);
        case ',': return create_token(TOKEN_COMMA, ",", lexer->line, lexer->start_column);
        case '.': return create_token(TOKEN_DOT, ".", lexer->line, lexer->start_column);
        case ':': return create_token(TOKEN_COLON, ":", lexer->line, lexer->start_column);
        case '#': return create_token(TOKEN_HASH, "#", lexer->line, lexer->start_column);
        case '+': return create_token(TOKEN_PLUS, "+", lexer->line, lexer->start_column);
        case '-': return create_token(TOKEN_MINUS, "-", lexer->line, lexer->start_column);
        case '*': return create_token(TOKEN_STAR, "*", lexer->line, lexer->start_column);
        case '/': return create_token(TOKEN_SLASH, "/", lexer->line, lexer->start_column);
        case '%': return create_token(TOKEN_PERCENT, "%", lexer->line, lexer->start_column);
        case '=':
            if (match(lexer, '=')) {
                return create_token(TOKEN_EQUAL, "==", lexer->line, lexer->start_column);
            }
            return create_token(TOKEN_ASSIGN, "=", lexer->line, lexer->start_column);
        case '!':
            if (match(lexer, '=')) {
                return create_token(TOKEN_NOT_EQUAL, "!=", lexer->line, lexer->start_column);
            }
            return create_token(TOKEN_NOT, "!", lexer->line, lexer->start_column);
        case '<':
            if (match(lexer, '=')) {
                return create_token(TOKEN_LESS_EQUAL, "<=", lexer->line, lexer->start_column);
            }
            return create_token(TOKEN_LESS, "<", lexer->line, lexer->start_column);
        case '>':
            if (match(lexer, '=')) {
                return create_token(TOKEN_GREATER_EQUAL, ">=", lexer->line, lexer->start_column);
            }
            return create_token(TOKEN_GREATER, ">", lexer->line, lexer->start_column);
    }
    
    char error_msg[50];
    sprintf(error_msg, "Unexpected character: %c", c);
    return create_token(TOKEN_ERROR, error_msg, lexer->line, lexer->start_column);
}

// Tokenize entire source
Token** lexer_tokenize(const char* source, int* token_count) {
    Lexer* lexer = lexer_init(source);
    
    int capacity = 256;
    Token** tokens = (Token**)malloc(sizeof(Token*) * capacity);
    *token_count = 0;
    
    while (true) {
        Token* token = lexer_next_token(lexer);
        
        // Skip newlines for simplicity (can be added back for statement separation)
        if (token->type == TOKEN_NEWLINE) {
            free_token(token);
            continue;
        }
        
        if (*token_count >= capacity) {
            capacity *= 2;
            tokens = (Token**)realloc(tokens, sizeof(Token*) * capacity);
        }
        
        tokens[(*token_count)++] = token;
        
        if (token->type == TOKEN_EOF || token->type == TOKEN_ERROR) {
            break;
        }
    }
    
    free(lexer);
    return tokens;
}

// Free lexer
void lexer_free(Lexer* lexer) {
    free(lexer);
}