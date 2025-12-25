#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
// main.c for the Kitler programming language

// External function declarations
extern Lexer* lexer_init(const char* source);
extern Token** lexer_tokenize(const char* source, int* token_count);
extern void lexer_free(Lexer* lexer);

extern Parser* parser_init(Token** tokens, int token_count);
extern ASTNode* parser_parse(Parser* parser);

extern Interpreter* interpreter_init();
extern void interpreter_run(Interpreter* interp, ASTNode* ast);
extern void interpreter_free(Interpreter* interp);

// Read file contents
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

// Run KT source code
int run_source(const char* source) {
    // Tokenize
    int token_count = 0;
    Token** tokens = lexer_tokenize(source, &token_count);
    
    printf("=== LEXER OUTPUT ===\n");
    printf("Generated %d tokens\n\n", token_count);
    
    // Parse
    Parser* parser = parser_init(tokens, token_count);
    ASTNode* ast = parser_parse(parser);
    
    if (parser->had_error) {
        fprintf(stderr, "Parse errors occurred.\n");
        free_ast(ast);
        free(parser);
        return 1;
    }
    
    printf("=== PARSER OUTPUT ===\n");
    printf("AST generated successfully\n\n");
    
    // Interpret
    printf("=== EXECUTION OUTPUT ===\n");
    Interpreter* interp = interpreter_init();
    interpreter_run(interp, ast);
    
    // Cleanup
    interpreter_free(interp);
    free_ast(ast);
    free(parser);
    
    for (int i = 0; i < token_count; i++) {
        free_token(tokens[i]);
    }
    free(tokens);
    
    return 0;
}

// Run file
int run_file(const char* filename) {
    char* source = read_file(filename);
    if (!source) return 1;
    
    printf("Running: %s\n", filename);
    printf("=====================================\n\n");
    
    int result = run_source(source);
    free(source);
    
    return result;
}

// REPL (Read-Eval-Print-Loop)
void run_repl() {
    char line[1024];
    
    printf("Kitler (KT) REPL v1.0\n");
    printf("Type 'exit' to quit\n\n");
    
    while (true) {
        printf("kt> ");
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }
        
        if (strlen(line) == 0) continue;
        
        run_source(line);
        printf("\n");
    }
    
    printf("Goodbye!\n");
}

// Print usage
void print_usage() {
    printf("Kitler (KT) Programming Language v1.0\n");
    printf("Usage:\n");
    printf("  kt                        Start REPL\n");
    printf("  kt run --file=<file.kt>   Run a KT file\n");
    printf("  kt --config               Configure project (interactive)\n");
    printf("  kt --config=auto          Auto-configure project\n");
    printf("  kt new <project>          Create new project\n");
    printf("  kt build --project=<name> Build project\n");
    printf("  kt gui --file=<file.kt>   Open GUI interpreter\n");
    printf("\n");
}

// Create new project
void create_project(const char* name) {
    printf("Creating new project: %s\n", name);
    
    // Create directory (Windows vs Unix)
    char mkdir_cmd[256];
#ifdef _WIN32
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "if not exist \"%s\\template\" mkdir \"%s\\template\"", name, name);
#else
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s/template", name);
#endif
    system(mkdir_cmd);
    
    // Create ktconfig
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "%s/%s.ktconfig", name, name);
    
    FILE* config = fopen(config_path, "w");
    fprintf(config, "{\n");
    fprintf(config, "  \"projectName\": \"%s\",\n", name);
    fprintf(config, "  \"dotnetVersion\": \"8\",\n");
    fprintf(config, "  \"projectType\": \"game\",\n");
    fprintf(config, "  \"autoOptimized\": true,\n");
    fprintf(config, "  \"includes\": [\n");
    fprintf(config, "    \"System.Interface\",\n");
    fprintf(config, "    \"Windows.NET8\"\n");
    fprintf(config, "  ],\n");
    fprintf(config, "  \"entryPoint\": \"Template.kt\"\n");
    fprintf(config, "}\n");
    fclose(config);
    
    // Create Template.kt
    char template_path[256];
    snprintf(template_path, sizeof(template_path), "%s/template/Template.kt", name);
    
    FILE* template = fopen(template_path, "w");
    fprintf(template, "including System.Interface#\n");
    fprintf(template, "including Windows.NET8#\n\n");
    fprintf(template, "projectSpace %s [\n", name);
    fprintf(template, "    %s.WhenRan[\n", name);
    fprintf(template, "        StartAll.Components()\n");
    fprintf(template, "        App.New = New WindowComponent(\"%s\", false, false, Windowed, 1280x720)\n", name);
    fprintf(template, "    ]\n");
    fprintf(template, "    \n");
    fprintf(template, "    NewFunc Initialize() (\n");
    fprintf(template, "        Console.Write(\"Hello from %s!\")\n", name);
    fprintf(template, "    )\n");
    fprintf(template, "    \n");
    fprintf(template, "    %s.Update[\n", name);
    fprintf(template, "        <-- Game loop goes here -->\n");
    fprintf(template, "    ]\n");
    fprintf(template, "    \n");
    fprintf(template, "    %s.Draw[\n", name);
    fprintf(template, "        <-- Drawing code goes here -->\n");
    fprintf(template, "    ]\n");
    fprintf(template, "]\n");
    fclose(template);
    
    printf("Project created successfully!\n");
    printf("  Config: %s\n", config_path);
    printf("  Template: %s\n", template_path);
    printf("\nRun with: kt run --file=%s/template/Template.kt\n", name);
}

// Main entry point
int main(int argc, char** argv) {
    if (argc == 1) {
        run_repl();
        return 0;
    }
    
    if (strcmp(argv[1], "run") == 0 && argc >= 3) {
        if (strncmp(argv[2], "--file=", 7) == 0) {
            return run_file(argv[2] + 7);
        }
    }
    
    if (strcmp(argv[1], "new") == 0 && argc >= 3) {
        create_project(argv[2]);
        return 0;
    }
    
    if (strcmp(argv[1], "--config") == 0) {
        printf("Interactive configuration not yet implemented.\n");
        printf("Use: kt --config=auto for auto-configuration\n");
        return 0;
    }
    
    if (strcmp(argv[1], "--config=auto") == 0) {
        printf("Auto-configuration not yet implemented.\n");
        return 0;
    }
    
    if (strcmp(argv[1], "gui") == 0) {
        printf("GUI interpreter not yet implemented.\n");
        printf("This will launch the C-based GUI editor.\n");
        return 0;
    }
    
    print_usage();
    return 0;
}

// Example KT code to test
const char* test_code = 
    "NewVar x = 10\n"
    "NewVar y = 20\n"
    "NewVar result = x + y\n"
    "Console.Write(\"Result:\", result)\n"
    "\n"
    "NewFunc greet(name) (\n"
    "    Console.Write(\"Hello\", name)\n"
    ")\n"
    "\n"
    "greet(\"World\")\n"
    "\n"
    "if result > 25 run:\n"
    "    Console.Write(\"Large number!\")\n"
    "end\n";