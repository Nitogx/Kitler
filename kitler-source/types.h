#ifndef KT_TYPES_H
#define KT_TYPES_H

#include <stddef.h>
#include <stdbool.h>
// types.h for the Kitler programming language

// Token types for the lexer
typedef enum {
    // Literals
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    
    // Keywords
    TOKEN_INCLUDING,
    TOKEN_PROJECTSPACE,
    TOKEN_NEWVAR,
    TOKEN_NEWFUNC,
    TOKEN_NEWCLASS,
    TOKEN_NEWEVENT,
    TOKEN_NEWASYNC,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_FOREACH,
    TOKEN_IN,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_BREAK,
    TOKEN_RETURN,
    TOKEN_RUN,
    TOKEN_END,
    TOKEN_WHEN,
    TOKEN_THIS,
    TOKEN_NEW,
    TOKEN_AWAIT,
    
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    
    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_HASH,
    
    // Special
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
    union {
        double number;
        char* string;
    } value;
} Token;

// AST Node types
typedef enum {
    NODE_PROGRAM,
    NODE_INCLUDING,
    NODE_PROJECTSPACE,
    NODE_WHENRAN,
    NODE_UPDATE,
    NODE_DRAW,
    NODE_ONEXIT,
    NODE_VARDECL,
    NODE_FUNCDECL,
    NODE_CLASSDECL,
    NODE_EVENTDECL,
    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_FOREACH,
    NODE_SWITCH,
    NODE_CASE,
    NODE_RETURN,
    NODE_BREAK,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_CALL,
    NODE_MEMBER_ACCESS,
    NODE_INDEX_ACCESS,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_LIST,
    NODE_MAP,
    NODE_NEW_INSTANCE
} NodeType;

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct Scope Scope;
typedef struct Value Value;

// AST Node structure
struct ASTNode {
    NodeType type;
    int line;
    int column;
    
    union {
        // Including directive
        struct {
            char* library;
            bool is_priority;
        } including;
        
        // ProjectSpace
        struct {
            char* name;
            ASTNode** children;
            int child_count;
        } projectspace;
        
        // Variable declaration
        struct {
            char* name;
            ASTNode* initializer;
        } var_decl;
        
        // Function declaration
        struct {
            char* name;
            char** params;
            int param_count;
            ASTNode* body;
            bool is_async;
        } func_decl;
        
        // Class declaration
        struct {
            char* name;
            ASTNode** members;
            int member_count;
        } class_decl;
        
        // Event declaration
        struct {
            char* name;
            char** params;
            int param_count;
        } event_decl;
        
        // Block statement
        struct {
            ASTNode** statements;
            int statement_count;
        } block;
        
        // If statement
        struct {
            ASTNode* condition;
            ASTNode* then_branch;
            ASTNode* else_branch;
        } if_stmt;
        
        // While loop
        struct {
            ASTNode* condition;
            ASTNode* body;
        } while_loop;
        
        // For loop
        struct {
            char* iterator;
            ASTNode* iterable;
            ASTNode* body;
        } for_loop;
        
        // Switch statement
        struct {
            ASTNode* expression;
            ASTNode** cases;
            int case_count;
            ASTNode* default_case;
        } switch_stmt;
        
        // Case statement
        struct {
            ASTNode* value;
            ASTNode* body;
        } case_stmt;
        
        // Assignment
        struct {
            ASTNode* target;
            ASTNode* value;
        } assignment;
        
        // Binary operation
        struct {
            TokenType operator;
            ASTNode* left;
            ASTNode* right;
        } binary_op;
        
        // Unary operation
        struct {
            TokenType operator;
            ASTNode* operand;
        } unary_op;
        
        // Function call
        struct {
            ASTNode* callee;
            ASTNode** args;
            int arg_count;
        } call;
        
        // Member access (obj.member)
        struct {
            ASTNode* object;
            char* member;
        } member_access;
        
        // Index access (arr[index])
        struct {
            ASTNode* object;
            ASTNode* index;
        } index_access;
        
        // Literal values
        struct {
            enum {
                LITERAL_NUMBER,
                LITERAL_STRING,
                LITERAL_BOOL,
                LITERAL_NULL
            } literal_type;
            union {
                double number;
                char* string;
                bool boolean;
            } literal_value;
        } literal;
        
        // Identifier
        struct {
            char* name;
        } identifier;
        
        // List literal
        struct {
            ASTNode** elements;
            int element_count;
        } list;
        
        // Map literal
        struct {
            char** keys;
            ASTNode** values;
            int pair_count;
        } map;
        
        // New instance
        struct {
            char* class_name;
            ASTNode** args;
            int arg_count;
        } new_instance;
        
        // Return statement
        struct {
            ASTNode* value;
        } return_stmt;
    } data;
};

// Value types for runtime
typedef enum {
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOL,
    VALUE_NULL,
    VALUE_LIST,
    VALUE_MAP,
    VALUE_FUNCTION,
    VALUE_CLASS,
    VALUE_INSTANCE,
    VALUE_NATIVE_FUNCTION,
    VALUE_SPRITE,
    VALUE_COMPONENT
} ValueType;

// Runtime value structure
struct Value {
    ValueType type;
    bool is_marked; // For garbage collection
    
    union {
        double number;
        char* string;
        bool boolean;
        
        struct {
            Value** elements;
            int count;
            int capacity;
        } list;
        
        struct {
            char** keys;
            Value** values;
            int count;
            int capacity;
        } map;
        
        struct {
            char* name;
            char** params;
            int param_count;
            ASTNode* body;
            Scope* closure;
        } function;
        
        struct {
            char* name;
            struct Value** methods;
            char** method_names;
            int method_count;
        } class_obj;
        
        struct {
            struct Value* class_ref;
            struct Value* fields; // Map of field values
        } instance;
        
        struct {
            char* name;
            Value* (*native_fn)(Value** args, int arg_count);
        } native_function;
        
        struct {
            void* sprite_data;
            double x, y;
            double width, height;
            double velocity_x, velocity_y;
        } sprite;
        
        struct {
            void* component_data;
            char* component_type;
        } component;
    } data;
};

// Scope structure for variable resolution
struct Scope {
    char** names;
    Value** values;
    int count;
    int capacity;
    Scope* parent;
};

// Lexer structure
typedef struct {
    const char* source;
    const char* current;
    int line;
    int column;
    int start_column;
} Lexer;

// Parser structure
typedef struct {
    Token* tokens;
    int token_count;
    int current;
    bool had_error;
    bool panic_mode;
} Parser;

// Interpreter structure
typedef struct {
    ASTNode* ast;
    Scope* global_scope;
    Scope* current_scope;
    Value** gc_objects;
    int gc_count;
    int gc_capacity;
    bool should_exit;
    Value* return_value;
} Interpreter;

// Function prototypes for memory management
Token* create_token(TokenType type, const char* lexeme, int line, int column);
void free_token(Token* token);
ASTNode* create_node(NodeType type, int line, int column);
void free_ast(ASTNode* node);
Value* create_value(ValueType type);
void free_value(Value* value);
Scope* create_scope(Scope* parent);
void free_scope(Scope* scope);

#endif // KT_TYPES_H