#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
// interpreter.c for Kitler

// Forward declarations
static Value* eval_node(Interpreter* interp, ASTNode* node);
static Value* eval_expression(Interpreter* interp, ASTNode* node);

// ============================================================================
// SCOPE HELPER FUNCTIONS
// (create_scope is NOT here; it is in types.c)
// ============================================================================

void scope_define(Scope* scope, const char* name, Value* value) {
    // Check if already defined in current scope
    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->names[i], name) == 0) {
            scope->values[i] = value;
            return;
        }
    }
    
    if (scope->count >= scope->capacity) {
        scope->capacity *= 2;
        scope->names = (char**)realloc(scope->names, sizeof(char*) * scope->capacity);
        scope->values = (Value**)realloc(scope->values, sizeof(Value*) * scope->capacity);
    }
    
    scope->names[scope->count] = strdup(name);
    scope->values[scope->count] = value;
    scope->count++;
}

Value* scope_get(Scope* scope, const char* name) {
    // Search current scope
    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->names[i], name) == 0) {
            return scope->values[i];
        }
    }
    
    // Search parent scope
    if (scope->parent) {
        return scope_get(scope->parent, name);
    }
    
    return NULL;
}

void scope_set(Scope* scope, const char* name, Value* value) {
    // Search current scope
    for (int i = 0; i < scope->count; i++) {
        if (strcmp(scope->names[i], name) == 0) {
            scope->values[i] = value;
            return;
        }
    }
    
    // Search parent scope
    if (scope->parent) {
        scope_set(scope->parent, name, value);
    }
}

// ============================================================================
// INTERPRETER CORE
// ============================================================================

// Initialize interpreter
Interpreter* interpreter_init() {
    Interpreter* interp = (Interpreter*)malloc(sizeof(Interpreter));
    interp->global_scope = create_scope(NULL); // Calls function from types.c
    interp->current_scope = interp->global_scope;
    interp->gc_capacity = 256;
    interp->gc_objects = (Value**)malloc(sizeof(Value*) * interp->gc_capacity);
    interp->gc_count = 0;
    interp->should_exit = false;
    interp->return_value = NULL;
    return interp;
}

// Register value for garbage collection
void gc_register(Interpreter* interp, Value* value) {
    if (interp->gc_count >= interp->gc_capacity) {
        interp->gc_capacity *= 2;
        interp->gc_objects = (Value**)realloc(
            interp->gc_objects, sizeof(Value*) * interp->gc_capacity);
    }
    interp->gc_objects[interp->gc_count++] = value;
}

// Built-in functions
static Value* builtin_print(Value** args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        Value* arg = args[i];
        
        switch (arg->type) {
            case VALUE_NUMBER:
                printf("%g", arg->data.number);
                break;
            case VALUE_STRING:
                printf("%s", arg->data.string);
                break;
            case VALUE_BOOL:
                printf("%s", arg->data.boolean ? "true" : "false");
                break;
            case VALUE_NULL:
                printf("null");
                break;
            default:
                printf("<object>");
                break;
        }
        
        if (i < arg_count - 1) printf(" ");
    }
    printf("\n");
    
    Value* result = create_value(VALUE_NULL);
    return result;
}

static Value* builtin_max(Value** args, int arg_count) {
    if (arg_count == 0) return create_value(VALUE_NULL);
    
    double max_val = args[0]->data.number;
    for (int i = 1; i < arg_count; i++) {
        if (args[i]->data.number > max_val) {
            max_val = args[i]->data.number;
        }
    }
    
    Value* result = create_value(VALUE_NUMBER);
    result->data.number = max_val;
    return result;
}

static Value* builtin_min(Value** args, int arg_count) {
    if (arg_count == 0) return create_value(VALUE_NULL);
    
    double min_val = args[0]->data.number;
    for (int i = 1; i < arg_count; i++) {
        if (args[i]->data.number < min_val) {
            min_val = args[i]->data.number;
        }
    }
    
    Value* result = create_value(VALUE_NUMBER);
    result->data.number = min_val;
    return result;
}

// Register built-in functions
void register_builtins(Interpreter* interp) {
    // Console.Write
    Value* print_fn = create_value(VALUE_NATIVE_FUNCTION);
    print_fn->data.native_function.name = strdup("Console.Write");
    print_fn->data.native_function.native_fn = builtin_print;
    scope_define(interp->global_scope, "Console.Write", print_fn);
    gc_register(interp, print_fn);
    
    // Max
    Value* max_fn = create_value(VALUE_NATIVE_FUNCTION);
    max_fn->data.native_function.name = strdup("Max");
    max_fn->data.native_function.native_fn = builtin_max;
    scope_define(interp->global_scope, "Max", max_fn);
    gc_register(interp, max_fn);
    
    // Min
    Value* min_fn = create_value(VALUE_NATIVE_FUNCTION);
    min_fn->data.native_function.name = strdup("Min");
    min_fn->data.native_function.native_fn = builtin_min;
    scope_define(interp->global_scope, "Min", min_fn);
    gc_register(interp, min_fn);
}

// Evaluate literal
static Value* eval_literal(Interpreter* interp, ASTNode* node) {
    Value* value = create_value(VALUE_NULL);
    
    switch (node->data.literal.literal_type) {
        case LITERAL_NUMBER:
            value->type = VALUE_NUMBER;
            value->data.number = node->data.literal.literal_value.number;
            break;
        case LITERAL_STRING:
            value->type = VALUE_STRING;
            value->data.string = strdup(node->data.literal.literal_value.string);
            break;
        case LITERAL_BOOL:
            value->type = VALUE_BOOL;
            value->data.boolean = node->data.literal.literal_value.boolean;
            break;
        case LITERAL_NULL:
            value->type = VALUE_NULL;
            break;
    }
    
    gc_register(interp, value);
    return value;
}

// Evaluate identifier
static Value* eval_identifier(Interpreter* interp, ASTNode* node) {
    Value* value = scope_get(interp->current_scope, node->data.identifier.name);
    
    if (!value) {
        fprintf(stderr, "Undefined variable: %s\n", node->data.identifier.name);
        return create_value(VALUE_NULL);
    }
    
    return value;
}

// Evaluate binary operation
static Value* eval_binary_op(Interpreter* interp, ASTNode* node) {
    Value* left = eval_expression(interp, node->data.binary_op.left);
    Value* right = eval_expression(interp, node->data.binary_op.right);
    
    Value* result = create_value(VALUE_NUMBER);
    gc_register(interp, result);
    
    switch (node->data.binary_op.operator) {
        case TOKEN_PLUS:
            if (left->type == VALUE_STRING || right->type == VALUE_STRING) {
                result->type = VALUE_STRING;
                char buffer[1024];
                snprintf(buffer, 1024, "%s%s", 
                    left->type == VALUE_STRING ? left->data.string : "",
                    right->type == VALUE_STRING ? right->data.string : "");
                result->data.string = strdup(buffer);
            } else {
                result->data.number = left->data.number + right->data.number;
            }
            break;
        case TOKEN_MINUS:
            result->data.number = left->data.number - right->data.number;
            break;
        case TOKEN_STAR:
            result->data.number = left->data.number * right->data.number;
            break;
        case TOKEN_SLASH:
            result->data.number = left->data.number / right->data.number;
            break;
        case TOKEN_PERCENT:
            result->data.number = fmod(left->data.number, right->data.number);
            break;
        case TOKEN_EQUAL:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number == right->data.number);
            break;
        case TOKEN_NOT_EQUAL:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number != right->data.number);
            break;
        case TOKEN_LESS:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number < right->data.number);
            break;
        case TOKEN_LESS_EQUAL:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number <= right->data.number);
            break;
        case TOKEN_GREATER:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number > right->data.number);
            break;
        case TOKEN_GREATER_EQUAL:
            result->type = VALUE_BOOL;
            result->data.boolean = (left->data.number >= right->data.number);
            break;
        case TOKEN_AND:
            result->type = VALUE_BOOL;
            result->data.boolean = left->data.boolean && right->data.boolean;
            break;
        case TOKEN_OR:
            result->type = VALUE_BOOL;
            result->data.boolean = left->data.boolean || right->data.boolean;
            break;
        default:
            break;
    }
    
    return result;
}

// Evaluate function call
static Value* eval_call(Interpreter* interp, ASTNode* node) {
    Value* callee = eval_expression(interp, node->data.call.callee);
    
    // Evaluate arguments
    Value** args = (Value**)malloc(sizeof(Value*) * node->data.call.arg_count);
    for (int i = 0; i < node->data.call.arg_count; i++) {
        args[i] = eval_expression(interp, node->data.call.args[i]);
    }
    
    Value* result = NULL;
    
    if (callee->type == VALUE_NATIVE_FUNCTION) {
        result = callee->data.native_function.native_fn(args, node->data.call.arg_count);
    } else if (callee->type == VALUE_FUNCTION) {
        // Create new scope for function
        Scope* func_scope = create_scope(callee->data.function.closure);
        
        // Bind parameters
        for (int i = 0; i < callee->data.function.param_count && i < node->data.call.arg_count; i++) {
            scope_define(func_scope, callee->data.function.params[i], args[i]);
        }
        
        // Execute function body
        Scope* prev_scope = interp->current_scope;
        interp->current_scope = func_scope;
        
        eval_node(interp, callee->data.function.body);
        
        result = interp->return_value ? interp->return_value : create_value(VALUE_NULL);
        interp->return_value = NULL;
        
        interp->current_scope = prev_scope;
        free_scope(func_scope);
    }
    
    free(args);
    return result ? result : create_value(VALUE_NULL);
}

// Evaluate expression
static Value* eval_expression(Interpreter* interp, ASTNode* node) {
    switch (node->type) {
        case NODE_LITERAL:
            return eval_literal(interp, node);
        case NODE_IDENTIFIER:
            return eval_identifier(interp, node);
        case NODE_BINARY_OP:
            return eval_binary_op(interp, node);
        case NODE_CALL:
            return eval_call(interp, node);
        default:
            return create_value(VALUE_NULL);
    }
}

// Evaluate variable declaration
static Value* eval_var_decl(Interpreter* interp, ASTNode* node) {
    Value* value = node->data.var_decl.initializer 
        ? eval_expression(interp, node->data.var_decl.initializer)
        : create_value(VALUE_NULL);
    
    scope_define(interp->current_scope, node->data.var_decl.name, value);
    return value;
}

// Evaluate function declaration
static Value* eval_func_decl(Interpreter* interp, ASTNode* node) {
    Value* func = create_value(VALUE_FUNCTION);
    func->data.function.name = strdup(node->data.func_decl.name);
    func->data.function.params = node->data.func_decl.params;
    func->data.function.param_count = node->data.func_decl.param_count;
    func->data.function.body = node->data.func_decl.body;
    func->data.function.closure = interp->current_scope;
    
    scope_define(interp->current_scope, node->data.func_decl.name, func);
    gc_register(interp, func);
    
    return func;
}

// Evaluate if statement
static Value* eval_if(Interpreter* interp, ASTNode* node) {
    Value* condition = eval_expression(interp, node->data.if_stmt.condition);
    
    if (condition->data.boolean) {
        return eval_node(interp, node->data.if_stmt.then_branch);
    } else if (node->data.if_stmt.else_branch) {
        return eval_node(interp, node->data.if_stmt.else_branch);
    }
    
    return create_value(VALUE_NULL);
}

// Evaluate while loop
static Value* eval_while(Interpreter* interp, ASTNode* node) {
    while (true) {
        Value* condition = eval_expression(interp, node->data.while_loop.condition);
        if (!condition->data.boolean) break;
        
        eval_node(interp, node->data.while_loop.body);
    }
    
    return create_value(VALUE_NULL);
}

// Evaluate assignment
static Value* eval_assignment(Interpreter* interp, ASTNode* node) {
    Value* value = eval_expression(interp, node->data.assignment.value);
    
    if (node->data.assignment.target->type == NODE_IDENTIFIER) {
        scope_set(interp->current_scope, 
            node->data.assignment.target->data.identifier.name, value);
    }
    
    return value;
}

// Evaluate block
static Value* eval_block(Interpreter* interp, ASTNode* node) {
    Value* result = create_value(VALUE_NULL);
    
    for (int i = 0; i < node->data.block.statement_count; i++) {
        result = eval_node(interp, node->data.block.statements[i]);
        
        if (interp->return_value) break;
    }
    
    return result;
}

// Evaluate AST node
static Value* eval_node(Interpreter* interp, ASTNode* node) {
    if (!node) return create_value(VALUE_NULL);
    
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            return eval_block(interp, node);
        case NODE_VARDECL:
            return eval_var_decl(interp, node);
        case NODE_FUNCDECL:
            return eval_func_decl(interp, node);
        case NODE_IF:
            return eval_if(interp, node);
        case NODE_WHILE:
            return eval_while(interp, node);
        case NODE_ASSIGN:
            return eval_assignment(interp, node);
        case NODE_RETURN:
            interp->return_value = node->data.return_stmt.value 
                ? eval_expression(interp, node->data.return_stmt.value)
                : create_value(VALUE_NULL);
            return interp->return_value;
        default:
            return eval_expression(interp, node);
    }
}

// Run interpreter
void interpreter_run(Interpreter* interp, ASTNode* ast) {
    interp->ast = ast;
    register_builtins(interp);
    eval_node(interp, ast);
}