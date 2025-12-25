#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
// Parser for Kitler

// Forward declarations
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser);
static ASTNode* parse_primary(Parser* parser);

// Initialize parser
Parser* parser_init(Token** tokens, int token_count) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->tokens = *tokens;
    parser->token_count = token_count;
    parser->current = 0;
    parser->had_error = false;
    parser->panic_mode = false;
    return parser;
}

// Peek current token
static Token* peek(Parser* parser) {
    return &parser->tokens[parser->current];
}

// Check if current token matches type
static bool check(Parser* parser, TokenType type) {
    if (parser->current >= parser->token_count) return false;
    return peek(parser)->type == type;
}

// Advance to next token
static Token* advance(Parser* parser) {
    if (parser->current < parser->token_count) {
        parser->current++;
    }
    return &parser->tokens[parser->current - 1];
}

// Match token type and advance
static bool match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

// Expect token type or error
static Token* expect(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) {
        return advance(parser);
    }
    
    Token* current = peek(parser);
    fprintf(stderr, "Parse error at line %d, column %d: %s\n", 
            current->line, current->column, message);
    parser->had_error = true;
    return NULL;
}

// Parse including directive
static ASTNode* parse_including(Parser* parser) {
    Token* including_token = advance(parser); // including
    Token* lib_name = expect(parser, TOKEN_IDENTIFIER, "Expected library name");
    
    if (!lib_name) return NULL;
    
    bool is_priority = false;
    if (match(parser, TOKEN_HASH)) {
        is_priority = true;
    }
    
    ASTNode* node = create_node(NODE_INCLUDING, including_token->line, including_token->column);
    node->data.including.library = strdup(lib_name->lexeme);
    node->data.including.is_priority = is_priority;
    
    return node;
}

// Parse block of statements
static ASTNode* parse_block(Parser* parser) {
    ASTNode* block = create_node(NODE_BLOCK, peek(parser)->line, peek(parser)->column);
    
    int capacity = 16;
    block->data.block.statements = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
    block->data.block.statement_count = 0;
    
    while (!check(parser, TOKEN_END) && !check(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            if (block->data.block.statement_count >= capacity) {
                capacity *= 2;
                block->data.block.statements = (ASTNode**)realloc(
                    block->data.block.statements, sizeof(ASTNode*) * capacity);
            }
            block->data.block.statements[block->data.block.statement_count++] = stmt;
        }
    }
    
    return block;
}

// Parse variable declaration
static ASTNode* parse_var_decl(Parser* parser) {
    Token* newvar_token = advance(parser); // NewVar
    Token* name = expect(parser, TOKEN_IDENTIFIER, "Expected variable name");
    
    if (!name) return NULL;
    
    ASTNode* node = create_node(NODE_VARDECL, newvar_token->line, newvar_token->column);
    node->data.var_decl.name = strdup(name->lexeme);
    node->data.var_decl.initializer = NULL;
    
    if (match(parser, TOKEN_ASSIGN)) {
        node->data.var_decl.initializer = parse_expression(parser);
    }
    
    return node;
}

// Parse function parameters
static void parse_params(Parser* parser, char*** params, int* param_count) {
    expect(parser, TOKEN_LPAREN, "Expected '(' after function name");
    
    int capacity = 8;
    *params = (char**)malloc(sizeof(char*) * capacity);
    *param_count = 0;
    
    if (!check(parser, TOKEN_RPAREN)) {
        do {
            Token* param = expect(parser, TOKEN_IDENTIFIER, "Expected parameter name");
            if (param) {
                if (*param_count >= capacity) {
                    capacity *= 2;
                    *params = (char**)realloc(*params, sizeof(char*) * capacity);
                }
                (*params)[(*param_count)++] = strdup(param->lexeme);
            }
        } while (match(parser, TOKEN_COMMA));
    }
    
    expect(parser, TOKEN_RPAREN, "Expected ')' after parameters");
}

// Parse function declaration
static ASTNode* parse_func_decl(Parser* parser) {
    Token* func_token = advance(parser); // NewFunc or NewAsync
    bool is_async = (func_token->type == TOKEN_NEWASYNC);
    
    Token* name = expect(parser, TOKEN_IDENTIFIER, "Expected function name");
    if (!name) return NULL;
    
    ASTNode* node = create_node(NODE_FUNCDECL, func_token->line, func_token->column);
    node->data.func_decl.name = strdup(name->lexeme);
    node->data.func_decl.is_async = is_async;
    
    parse_params(parser, &node->data.func_decl.params, &node->data.func_decl.param_count);
    
    expect(parser, TOKEN_LPAREN, "Expected '(' before function body");
    node->data.func_decl.body = parse_block(parser);
    expect(parser, TOKEN_RPAREN, "Expected ')' after function body");
    
    return node;
}

// Parse if statement
static ASTNode* parse_if(Parser* parser) {
    Token* if_token = advance(parser); // if
    
    ASTNode* node = create_node(NODE_IF, if_token->line, if_token->column);
    node->data.if_stmt.condition = parse_expression(parser);
    
    expect(parser, TOKEN_RUN, "Expected 'run:' after if condition");
    expect(parser, TOKEN_COLON, "Expected ':' after 'run'");
    
    node->data.if_stmt.then_branch = parse_block(parser);
    
    if (match(parser, TOKEN_END)) {
        node->data.if_stmt.else_branch = NULL;
    } else if (match(parser, TOKEN_ELSE)) {
        expect(parser, TOKEN_COLON, "Expected ':' after 'else'");
        node->data.if_stmt.else_branch = parse_block(parser);
        expect(parser, TOKEN_END, "Expected 'end' after else block");
    }
    
    return node;
}

// Parse while loop
static ASTNode* parse_while(Parser* parser) {
    Token* while_token = advance(parser); // while
    
    ASTNode* node = create_node(NODE_WHILE, while_token->line, while_token->column);
    node->data.while_loop.condition = parse_expression(parser);
    
    expect(parser, TOKEN_RUN, "Expected 'run:' after while condition");
    expect(parser, TOKEN_COLON, "Expected ':' after 'run'");
    
    node->data.while_loop.body = parse_block(parser);
    expect(parser, TOKEN_END, "Expected 'end' after while block");
    
    return node;
}

// Parse for/foreach loop
static ASTNode* parse_for(Parser* parser) {
    Token* for_token = advance(parser); // for or foreach
    
    Token* iterator = expect(parser, TOKEN_IDENTIFIER, "Expected iterator variable");
    if (!iterator) return NULL;
    
    expect(parser, TOKEN_IN, "Expected 'in' after iterator");
    
    ASTNode* node = create_node(NODE_FOR, for_token->line, for_token->column);
    node->data.for_loop.iterator = strdup(iterator->lexeme);
    node->data.for_loop.iterable = parse_expression(parser);
    
    expect(parser, TOKEN_RUN, "Expected 'run:' after for condition");
    expect(parser, TOKEN_COLON, "Expected ':' after 'run'");
    
    node->data.for_loop.body = parse_block(parser);
    expect(parser, TOKEN_END, "Expected 'end' after for block");
    
    return node;
}

// Parse return statement
static ASTNode* parse_return(Parser* parser) {
    Token* return_token = advance(parser); // return
    
    ASTNode* node = create_node(NODE_RETURN, return_token->line, return_token->column);
    
    if (!check(parser, TOKEN_END) && !check(parser, TOKEN_EOF)) {
        node->data.return_stmt.value = parse_expression(parser);
    } else {
        node->data.return_stmt.value = NULL;
    }
    
    return node;
}

// Parse assignment or expression statement
static ASTNode* parse_assignment_or_expr(Parser* parser) {
    ASTNode* expr = parse_expression(parser);
    
    if (match(parser, TOKEN_ASSIGN)) {
        ASTNode* node = create_node(NODE_ASSIGN, peek(parser)->line, peek(parser)->column);
        node->data.assignment.target = expr;
        node->data.assignment.value = parse_expression(parser);
        return node;
    }
    
    return expr;
}

// Parse statement
static ASTNode* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_NEWVAR)) {
        parser->current--;
        return parse_var_decl(parser);
    }
    
    if (match(parser, TOKEN_NEWFUNC) || match(parser, TOKEN_NEWASYNC)) {
        parser->current--;
        return parse_func_decl(parser);
    }
    
    if (match(parser, TOKEN_IF)) {
        parser->current--;
        return parse_if(parser);
    }
    
    if (match(parser, TOKEN_WHILE)) {
        parser->current--;
        return parse_while(parser);
    }
    
    if (match(parser, TOKEN_FOR) || match(parser, TOKEN_FOREACH)) {
        parser->current--;
        return parse_for(parser);
    }
    
    if (match(parser, TOKEN_RETURN)) {
        parser->current--;
        return parse_return(parser);
    }
    
    if (match(parser, TOKEN_BREAK)) {
        return create_node(NODE_BREAK, peek(parser)->line, peek(parser)->column);
    }
    
    return parse_assignment_or_expr(parser);
}

// Parse binary expression (with operator precedence)
static ASTNode* parse_binary(Parser* parser, int min_precedence) {
    ASTNode* left = parse_primary(parser);
    
    while (true) {
        Token* op_token = peek(parser);
        int precedence = 0;
        
        switch (op_token->type) {
            case TOKEN_OR: precedence = 1; break;
            case TOKEN_AND: precedence = 2; break;
            case TOKEN_EQUAL:
            case TOKEN_NOT_EQUAL: precedence = 3; break;
            case TOKEN_LESS:
            case TOKEN_LESS_EQUAL:
            case TOKEN_GREATER:
            case TOKEN_GREATER_EQUAL: precedence = 4; break;
            case TOKEN_PLUS:
            case TOKEN_MINUS: precedence = 5; break;
            case TOKEN_STAR:
            case TOKEN_SLASH:
            case TOKEN_PERCENT: precedence = 6; break;
            default: return left;
        }
        
        if (precedence < min_precedence) return left;
        
        advance(parser);
        ASTNode* right = parse_binary(parser, precedence + 1);
        
        ASTNode* binary = create_node(NODE_BINARY_OP, op_token->line, op_token->column);
        binary->data.binary_op.operator = op_token->type;
        binary->data.binary_op.left = left;
        binary->data.binary_op.right = right;
        
        left = binary;
    }
    
    return left;
}

// Parse expression
static ASTNode* parse_expression(Parser* parser) {
    return parse_binary(parser, 0);
}

// Parse primary expression
static ASTNode* parse_primary(Parser* parser) {
    Token* token = peek(parser);
    
    // Number literal
    if (match(parser, TOKEN_NUMBER)) {
        ASTNode* node = create_node(NODE_LITERAL, token->line, token->column);
        node->data.literal.literal_type = LITERAL_NUMBER;
        node->data.literal.literal_value.number = token->value.number;
        return node;
    }
    
    // String literal
    if (match(parser, TOKEN_STRING)) {
        ASTNode* node = create_node(NODE_LITERAL, token->line, token->column);
        node->data.literal.literal_type = LITERAL_STRING;
        node->data.literal.literal_value.string = strdup(token->value.string);
        return node;
    }
    
    // Boolean literals
    if (match(parser, TOKEN_TRUE)) {
        ASTNode* node = create_node(NODE_LITERAL, token->line, token->column);
        node->data.literal.literal_type = LITERAL_BOOL;
        node->data.literal.literal_value.boolean = true;
        return node;
    }
    
    if (match(parser, TOKEN_FALSE)) {
        ASTNode* node = create_node(NODE_LITERAL, token->line, token->column);
        node->data.literal.literal_type = LITERAL_BOOL;
        node->data.literal.literal_value.boolean = false;
        return node;
    }
    
    // Identifier or function call
    if (match(parser, TOKEN_IDENTIFIER)) {
        ASTNode* node = create_node(NODE_IDENTIFIER, token->line, token->column);
        node->data.identifier.name = strdup(token->lexeme);
        
        // Check for function call
        if (match(parser, TOKEN_LPAREN)) {
            ASTNode* call = create_node(NODE_CALL, token->line, token->column);
            call->data.call.callee = node;
            
            int capacity = 8;
            call->data.call.args = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
            call->data.call.arg_count = 0;
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    if (call->data.call.arg_count >= capacity) {
                        capacity *= 2;
                        call->data.call.args = (ASTNode**)realloc(
                            call->data.call.args, sizeof(ASTNode*) * capacity);
                    }
                    call->data.call.args[call->data.call.arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            
            expect(parser, TOKEN_RPAREN, "Expected ')' after arguments");
            return call;
        }
        
        // Check for member access
        if (match(parser, TOKEN_DOT)) {
            Token* member = expect(parser, TOKEN_IDENTIFIER, "Expected member name");
            ASTNode* access = create_node(NODE_MEMBER_ACCESS, token->line, token->column);
            access->data.member_access.object = node;
            access->data.member_access.member = strdup(member->lexeme);
            return access;
        }
        
        return node;
    }
    
    // Parenthesized expression
    if (match(parser, TOKEN_LPAREN)) {
        ASTNode* expr = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    fprintf(stderr, "Unexpected token at line %d: %s\n", token->line, token->lexeme);
    parser->had_error = true;
    return NULL;
}

// Parse program
ASTNode* parser_parse(Parser* parser) {
    ASTNode* program = create_node(NODE_PROGRAM, 1, 1);
    
    int capacity = 32;
    program->data.block.statements = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
    program->data.block.statement_count = 0;
    
    while (!check(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        
        if (stmt) {
            if (program->data.block.statement_count >= capacity) {
                capacity *= 2;
                program->data.block.statements = (ASTNode**)realloc(
                    program->data.block.statements, sizeof(ASTNode*) * capacity);
            }
            program->data.block.statements[program->data.block.statement_count++] = stmt;
        }
        
        if (parser->had_error) break;
    }
    
    return program;
}