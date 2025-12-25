/*
 * types.c - Memory management for Kitler types
 */

#include "types.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// TOKEN MANAGEMENT
// ============================================================================

Token* create_token(TokenType type, const char* lexeme, int line, int column) {
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;
    token->value.number = 0;
    return token;
}

void free_token(Token* token) {
    if (!token) return;
    
    if (token->lexeme) {
        free(token->lexeme);
    }
    
    // Free string value if it's a string token
    if (token->type == TOKEN_STRING && token->value.string) {
        // Note: string might be shared with lexeme, check before freeing
        if (token->value.string != token->lexeme) {
            free(token->value.string);
        }
    }
    
    free(token);
}

// ============================================================================
// AST NODE MANAGEMENT
// ============================================================================

ASTNode* create_node(NodeType type, int line, int column) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_INCLUDING:
            if (node->data.including.library) free(node->data.including.library);
            break;
            
        case NODE_PROJECTSPACE:
            if (node->data.projectspace.name) free(node->data.projectspace.name);
            for (int i = 0; i < node->data.projectspace.child_count; i++) {
                free_ast(node->data.projectspace.children[i]);
            }
            if (node->data.projectspace.children) free(node->data.projectspace.children);
            break;
            
        case NODE_VARDECL:
            if (node->data.var_decl.name) free(node->data.var_decl.name);
            free_ast(node->data.var_decl.initializer);
            break;
            
        case NODE_FUNCDECL:
            if (node->data.func_decl.name) free(node->data.func_decl.name);
            for (int i = 0; i < node->data.func_decl.param_count; i++) {
                if (node->data.func_decl.params[i]) free(node->data.func_decl.params[i]);
            }
            if (node->data.func_decl.params) free(node->data.func_decl.params);
            free_ast(node->data.func_decl.body);
            break;
            
        case NODE_CLASSDECL:
            if (node->data.class_decl.name) free(node->data.class_decl.name);
            for (int i = 0; i < node->data.class_decl.member_count; i++) {
                free_ast(node->data.class_decl.members[i]);
            }
            if (node->data.class_decl.members) free(node->data.class_decl.members);
            break;
            
        case NODE_EVENTDECL:
            if (node->data.event_decl.name) free(node->data.event_decl.name);
            for (int i = 0; i < node->data.event_decl.param_count; i++) {
                if (node->data.event_decl.params[i]) free(node->data.event_decl.params[i]);
            }
            if (node->data.event_decl.params) free(node->data.event_decl.params);
            break;
            
        case NODE_BLOCK:
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                free_ast(node->data.block.statements[i]);
            }
            if (node->data.block.statements) free(node->data.block.statements);
            break;
            
        case NODE_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.then_branch);
            free_ast(node->data.if_stmt.else_branch);
            break;
            
        case NODE_WHILE:
            free_ast(node->data.while_loop.condition);
            free_ast(node->data.while_loop.body);
            break;
            
        case NODE_FOR:
        case NODE_FOREACH:
            if (node->data.for_loop.iterator) free(node->data.for_loop.iterator);
            free_ast(node->data.for_loop.iterable);
            free_ast(node->data.for_loop.body);
            break;
            
        case NODE_SWITCH:
            free_ast(node->data.switch_stmt.expression);
            for (int i = 0; i < node->data.switch_stmt.case_count; i++) {
                free_ast(node->data.switch_stmt.cases[i]);
            }
            if (node->data.switch_stmt.cases) free(node->data.switch_stmt.cases);
            free_ast(node->data.switch_stmt.default_case);
            break;
            
        case NODE_CASE:
            free_ast(node->data.case_stmt.value);
            free_ast(node->data.case_stmt.body);
            break;
            
        case NODE_ASSIGN:
            free_ast(node->data.assignment.target);
            free_ast(node->data.assignment.value);
            break;
            
        case NODE_BINARY_OP:
            free_ast(node->data.binary_op.left);
            free_ast(node->data.binary_op.right);
            break;
            
        case NODE_UNARY_OP:
            free_ast(node->data.unary_op.operand);
            break;
            
        case NODE_CALL:
            free_ast(node->data.call.callee);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                free_ast(node->data.call.args[i]);
            }
            if (node->data.call.args) free(node->data.call.args);
            break;
            
        case NODE_MEMBER_ACCESS:
            free_ast(node->data.member_access.object);
            if (node->data.member_access.member) free(node->data.member_access.member);
            break;
            
        case NODE_INDEX_ACCESS:
            free_ast(node->data.index_access.object);
            free_ast(node->data.index_access.index);
            break;
            
        case NODE_LITERAL:
            if (node->data.literal.literal_type == LITERAL_STRING && 
                node->data.literal.literal_value.string) {
                free(node->data.literal.literal_value.string);
            }
            break;
            
        case NODE_IDENTIFIER:
            if (node->data.identifier.name) free(node->data.identifier.name);
            break;
            
        case NODE_LIST:
            for (int i = 0; i < node->data.list.element_count; i++) {
                free_ast(node->data.list.elements[i]);
            }
            if (node->data.list.elements) free(node->data.list.elements);
            break;
            
        case NODE_MAP:
            for (int i = 0; i < node->data.map.pair_count; i++) {
                if (node->data.map.keys[i]) free(node->data.map.keys[i]);
                free_ast(node->data.map.values[i]);
            }
            if (node->data.map.keys) free(node->data.map.keys);
            if (node->data.map.values) free(node->data.map.values);
            break;
            
        case NODE_NEW_INSTANCE:
            if (node->data.new_instance.class_name) free(node->data.new_instance.class_name);
            for (int i = 0; i < node->data.new_instance.arg_count; i++) {
                free_ast(node->data.new_instance.args[i]);
            }
            if (node->data.new_instance.args) free(node->data.new_instance.args);
            break;
            
        case NODE_RETURN:
            free_ast(node->data.return_stmt.value);
            break;
            
        default:
            break;
    }
    
    free(node);
}

// ============================================================================
// VALUE MANAGEMENT
// ============================================================================

Value* create_value(ValueType type) {
    Value* value = (Value*)malloc(sizeof(Value));
    memset(value, 0, sizeof(Value));
    value->type = type;
    value->is_marked = false;
    return value;
}

void free_value(Value* value) {
    if (!value) return;
    
    switch (value->type) {
        case VALUE_STRING:
            if (value->data.string) free(value->data.string);
            break;
            
        case VALUE_LIST:
            for (int i = 0; i < value->data.list.count; i++) {
                free_value(value->data.list.elements[i]);
            }
            if (value->data.list.elements) free(value->data.list.elements);
            break;
            
        case VALUE_MAP:
            for (int i = 0; i < value->data.map.count; i++) {
                if (value->data.map.keys[i]) free(value->data.map.keys[i]);
                free_value(value->data.map.values[i]);
            }
            if (value->data.map.keys) free(value->data.map.keys);
            if (value->data.map.values) free(value->data.map.values);
            break;
            
        case VALUE_FUNCTION:
            if (value->data.function.name) free(value->data.function.name);
            // Note: params are managed by AST, don't free here
            // Note: body is managed by AST, don't free here
            break;
            
        case VALUE_NATIVE_FUNCTION:
            if (value->data.native_function.name) free(value->data.native_function.name);
            break;
            
        case VALUE_CLASS:
            if (value->data.class_obj.name) free(value->data.class_obj.name);
            for (int i = 0; i < value->data.class_obj.method_count; i++) {
                if (value->data.class_obj.method_names[i]) {
                    free(value->data.class_obj.method_names[i]);
                }
            }
            if (value->data.class_obj.method_names) free(value->data.class_obj.method_names);
            if (value->data.class_obj.methods) free(value->data.class_obj.methods);
            break;
            
        case VALUE_INSTANCE:
            free_value(value->data.instance.fields);
            break;
            
        case VALUE_SPRITE:
            // Sprite data would be freed here if allocated
            break;
            
        case VALUE_COMPONENT:
            if (value->data.component.component_type) {
                free(value->data.component.component_type);
            }
            break;
            
        default:
            break;
    }
    
    free(value);
}

// ============================================================================
// SCOPE MANAGEMENT
// ============================================================================

Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->capacity = 16;
    scope->names = (char**)malloc(sizeof(char*) * scope->capacity);
    scope->values = (Value**)malloc(sizeof(Value*) * scope->capacity);
    scope->count = 0;
    scope->parent = parent;
    return scope;
}

void free_scope(Scope* scope) {
    if (!scope) return;
    
    for (int i = 0; i < scope->count; i++) {
        if (scope->names[i]) free(scope->names[i]);
        // Note: values are managed by GC, don't free here
    }
    
    if (scope->names) free(scope->names);
    if (scope->values) free(scope->values);
    free(scope);
}

void interpreter_free(Interpreter* interp) {
    if (!interp) return;
    
    // Free all GC objects
    for (int i = 0; i < interp->gc_count; i++) {
        free_value(interp->gc_objects[i]);
    }
    if (interp->gc_objects) free(interp->gc_objects);
    
    // Free scopes
    free_scope(interp->global_scope);
    
    free(interp);
}