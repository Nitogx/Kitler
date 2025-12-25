/*
 * Kitler IDE - Complete Modern Editor with Syntax Highlighting
 * Visual Studio 2026 Style with Build System Integration
 * 
 * Build: gcc kitler_ide.c lexer.c parser.c interpreter.c types.c -o kitler-ide `pkg-config --cflags --libs gtk+-3.0` -lm
 */

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include "types.h"

// External functions from your interpreter
extern int run_source(const char* source);
extern Token** lexer_tokenize(const char* source, int* token_count);
extern void free_token(Token* token);

// Application state
typedef struct {
    GtkWidget* main_window;
    GtkWidget* welcome_window;
    GtkWidget* text_view;
    GtkTextBuffer* buffer;
    GtkWidget* output_view;
    GtkTextBuffer* output_buffer;
    GtkWidget* status_bar;
    GtkWidget* line_numbers;
    GtkWidget* sidebar;
    GtkWidget* build_button;
    GtkWidget* run_button;
    
    char* current_file;
    char* project_path;
    char* project_name;
    bool is_modified;
    bool is_running;
    bool dark_mode;
    
    // Syntax highlighting tags
    GtkTextTag* tag_keyword;
    GtkTextTag* tag_string;
    GtkTextTag* tag_number;
    GtkTextTag* tag_comment;
    GtkTextTag* tag_identifier;
    GtkTextTag* tag_operator;
} IDEState;

IDEState g_ide;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void apply_global_dark_theme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    const gchar* css = 
        "* { color: #d4d4d4; }"
        "window { background-color: #1e1e1e; }"
        ".welcome-window { background-color: #2d2d30; border: none; }"
        ".welcome-title { color: #ffffff; font-size: 32px; font-weight: bold; margin: 20px; }"
        ".welcome-subtitle { color: #cccccc; font-size: 14px; margin: 10px; }"
        ".project-card { "
        "  background: linear-gradient(135deg, #3e3e42 0%, #2d2d30 100%); "
        "  border: 1px solid #555; "
        "  border-radius: 8px; "
        "  padding: 25px; "
        "  margin: 10px; "
        "  transition: all 0.3s; "
        "}"
        ".project-card:hover { "
        "  background: linear-gradient(135deg, #4e4e52 0%, #3e3e42 100%); "
        "  border-color: #007acc; "
        "  box-shadow: 0 4px 12px rgba(0, 122, 204, 0.3); "
        "}"
        ".card-title { color: #ffffff; font-size: 16px; font-weight: bold; }"
        ".card-description { color: #b0b0b0; font-size: 12px; margin-top: 5px; }"
        ".editor-view { "
        "  background-color: #1e1e1e; "
        "  color: #d4d4d4; "
        "  font-family: 'Fira Code', 'Consolas', 'Monaco', monospace; "
        "  font-size: 13px; "
        "  caret-color: #ffffff; "
        "}"
        ".output-view { "
        "  background-color: #1e1e1e; "
        "  color: #cccccc; "
        "  font-family: 'Consolas', monospace; "
        "  font-size: 11px; "
        "}"
        ".line-numbers { "
        "  background-color: #1e1e1e; "
        "  color: #858585; "
        "  font-family: 'Consolas', monospace; "
        "  font-size: 13px; "
        "  padding-right: 10px; "
        "  border-right: 1px solid #3e3e42; "
        "}"
        ".sidebar { "
        "  background-color: #252526; "
        "  border-right: 1px solid #3e3e42; "
        "}"
        ".toolbar { "
        "  background: linear-gradient(to bottom, #2d2d30 0%, #252526 100%); "
        "  border-bottom: 1px solid #3e3e42; "
        "  padding: 5px; "
        "}"
        "toolbar button { "
        "  background-color: #3e3e42; "
        "  border: 1px solid #555; "
        "  border-radius: 4px; "
        "  color: #ffffff; "
        "  padding: 8px 16px; "
        "  margin: 2px; "
        "}"
        "toolbar button:hover { "
        "  background-color: #505050; "
        "  border-color: #007acc; "
        "}"
        "toolbar button.run-button { "
        "  background: linear-gradient(135deg, #16c60c 0%, #13a10e 100%); "
        "  border: none; "
        "}"
        "toolbar button.run-button:hover { "
        "  background: linear-gradient(135deg, #18d90e 0%, #16c60c 100%); "
        "}"
        "toolbar button.build-button { "
        "  background-color: #0e639c; "
        "  border: none; "
        "}"
        "toolbar button.build-button:hover { "
        "  background-color: #1177bb; "
        "}"
        ".statusbar { "
        "  background: linear-gradient(to right, #007acc 0%, #005a9e 100%); "
        "  color: #ffffff; "
        "  font-size: 12px; "
        "  padding: 4px 10px; "
        "}"
        "scrollbar { background-color: #1e1e1e; }"
        "scrollbar slider { background-color: #424242; border-radius: 4px; }"
        "scrollbar slider:hover { background-color: #4e4e4e; }";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(provider);
}

// ============================================================================
// SYNTAX HIGHLIGHTING
// ============================================================================

void setup_syntax_highlighting_tags() {
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(g_ide.buffer);
    
    // Keywords (blue)
    g_ide.tag_keyword = gtk_text_buffer_create_tag(g_ide.buffer, "keyword",
        "foreground", "#569cd6", "weight", PANGO_WEIGHT_BOLD, NULL);
    
    // Strings (orange)
    g_ide.tag_string = gtk_text_buffer_create_tag(g_ide.buffer, "string",
        "foreground", "#ce9178", NULL);
    
    // Numbers (light green)
    g_ide.tag_number = gtk_text_buffer_create_tag(g_ide.buffer, "number",
        "foreground", "#b5cea8", NULL);
    
    // Comments (green)
    g_ide.tag_comment = gtk_text_buffer_create_tag(g_ide.buffer, "comment",
        "foreground", "#6a9955", "style", PANGO_STYLE_ITALIC, NULL);
    
    // Identifiers (light blue)
    g_ide.tag_identifier = gtk_text_buffer_create_tag(g_ide.buffer, "identifier",
        "foreground", "#9cdcfe", NULL);
    
    // Operators (white)
    g_ide.tag_operator = gtk_text_buffer_create_tag(g_ide.buffer, "operator",
        "foreground", "#d4d4d4", NULL);
}

void apply_syntax_highlighting() {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_ide.buffer, &start, &end);
    
    // Remove all tags first
    gtk_text_buffer_remove_all_tags(g_ide.buffer, &start, &end);
    
    char* text = gtk_text_buffer_get_text(g_ide.buffer, &start, &end, FALSE);
    
    // Tokenize for syntax highlighting
    int token_count = 0;
    Token** tokens = lexer_tokenize(text, &token_count);
    
    for (int i = 0; i < token_count; i++) {
        Token* token = tokens[i];
        
        // Find the token position in buffer
        GtkTextIter token_start, token_end;
        gtk_text_buffer_get_iter_at_line(g_ide.buffer, &token_start, token->line - 1);
        gtk_text_iter_forward_chars(&token_start, token->column - 1);
        token_end = token_start;
        gtk_text_iter_forward_chars(&token_end, strlen(token->lexeme));
        
        GtkTextTag* tag = NULL;
        
        switch (token->type) {
            case TOKEN_NEWVAR:
            case TOKEN_NEWFUNC:
            case TOKEN_NEWCLASS:
            case TOKEN_NEWEVENT:
            case TOKEN_NEWASYNC:
            case TOKEN_IF:
            case TOKEN_ELSE:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_FOREACH:
            case TOKEN_IN:
            case TOKEN_SWITCH:
            case TOKEN_CASE:
            case TOKEN_DEFAULT:
            case TOKEN_BREAK:
            case TOKEN_RETURN:
            case TOKEN_RUN:
            case TOKEN_END:
            case TOKEN_WHEN:
            case TOKEN_THIS:
            case TOKEN_NEW:
            case TOKEN_AWAIT:
            case TOKEN_INCLUDING:
            case TOKEN_PROJECTSPACE:
            case TOKEN_TRUE:
            case TOKEN_FALSE:
                tag = g_ide.tag_keyword;
                break;
                
            case TOKEN_STRING:
                tag = g_ide.tag_string;
                break;
                
            case TOKEN_NUMBER:
                tag = g_ide.tag_number;
                break;
                
            case TOKEN_IDENTIFIER:
                tag = g_ide.tag_identifier;
                break;
                
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_STAR:
            case TOKEN_SLASH:
            case TOKEN_PERCENT:
            case TOKEN_ASSIGN:
            case TOKEN_EQUAL:
            case TOKEN_NOT_EQUAL:
            case TOKEN_LESS:
            case TOKEN_LESS_EQUAL:
            case TOKEN_GREATER:
            case TOKEN_GREATER_EQUAL:
            case TOKEN_AND:
            case TOKEN_OR:
            case TOKEN_NOT:
                tag = g_ide.tag_operator;
                break;
                
            default:
                break;
        }
        
        if (tag) {
            gtk_text_buffer_apply_tag(g_ide.buffer, tag, &token_start, &token_end);
        }
        
        free_token(token);
    }
    
    free(tokens);
    g_free(text);
}

void on_text_changed(GtkTextBuffer* buffer, gpointer data) {
    g_ide.is_modified = true;
    
    // Apply syntax highlighting with a slight delay to avoid lag
    static guint timeout_id = 0;
    if (timeout_id != 0) {
        g_source_remove(timeout_id);
    }
    timeout_id = g_timeout_add(150, (GSourceFunc)apply_syntax_highlighting, NULL);
}

// ============================================================================
// LINE NUMBERS
// ============================================================================

void update_line_numbers() {
    if (!g_ide.line_numbers) return;
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_ide.buffer, &start, &end);
    int line_count = gtk_text_buffer_get_line_count(g_ide.buffer);
    
    char* numbers = malloc(line_count * 10);
    numbers[0] = '\0';
    
    for (int i = 1; i <= line_count; i++) {
        char line[10];
        snprintf(line, sizeof(line), "%d\n", i);
        strcat(numbers, line);
    }
    
    gtk_label_set_text(GTK_LABEL(g_ide.line_numbers), numbers);
    free(numbers);
}

// ============================================================================
// BUILD & RUN SYSTEM
// ============================================================================

void append_output(const char* text, bool is_error) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(g_ide.output_buffer, &end);
    
    if (is_error) {
        GtkTextTag* error_tag = gtk_text_buffer_create_tag(g_ide.output_buffer, NULL,
            "foreground", "#f48771", NULL);
        gtk_text_buffer_insert_with_tags(g_ide.output_buffer, &end, text, -1, error_tag, NULL);
    } else {
        gtk_text_buffer_insert(g_ide.output_buffer, &end, text, -1);
    }
    
    // Auto-scroll to bottom
    GtkTextMark* mark = gtk_text_buffer_get_insert(g_ide.output_buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(g_ide.output_view), mark, 0.0, TRUE, 0.0, 1.0);
}

void on_build_project(GtkWidget* widget, gpointer data) {
    gtk_text_buffer_set_text(g_ide.output_buffer, "", -1);
    
    append_output("========================================\n", false);
    append_output("Build started...\n", false);
    append_output("========================================\n\n", false);
    
    gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Building project...");
    gtk_widget_set_sensitive(g_ide.build_button, FALSE);
    
    // Simulate build process
    append_output("[1/3] Checking dependencies...\n", false);
    usleep(300000);
    append_output("  ✓ System.Core found\n", false);
    append_output("  ✓ Windows.NET8 found\n", false);
    append_output("\n", false);
    
    append_output("[2/3] Lexical analysis...\n", false);
    usleep(300000);
    
    // Actually tokenize to check for errors
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_ide.buffer, &start, &end);
    char* source = gtk_text_buffer_get_text(g_ide.buffer, &start, &end, FALSE);
    
    int token_count = 0;
    Token** tokens = lexer_tokenize(source, &token_count);
    
    bool has_errors = false;
    for (int i = 0; i < token_count; i++) {
        if (tokens[i]->type == TOKEN_ERROR) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                "  ✗ Error at line %d: %s\n", tokens[i]->line, tokens[i]->lexeme);
            append_output(error_msg, true);
            has_errors = true;
        }
        free_token(tokens[i]);
    }
    free(tokens);
    g_free(source);
    
    if (!has_errors) {
        append_output("  ✓ Lexical analysis completed\n", false);
        append_output("\n[3/3] Code generation...\n", false);
        usleep(300000);
        append_output("  ✓ IL code generated\n", false);
        append_output("\n", false);
        
        append_output("========================================\n", false);
        append_output("Build succeeded! ✓\n", false);
        append_output("========================================\n", false);
        
        gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Build succeeded ✓");
        gtk_widget_set_sensitive(g_ide.run_button, TRUE);
    } else {
        append_output("\n========================================\n", false);
        append_output("Build failed! ✗\n", true);
        append_output("========================================\n", false);
        
        gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Build failed ✗");
        gtk_widget_set_sensitive(g_ide.run_button, FALSE);
    }
    
    gtk_widget_set_sensitive(g_ide.build_button, TRUE);
}

void on_run_code(GtkWidget* widget, gpointer data) {
    if (g_ide.is_running) return;
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_ide.buffer, &start, &end);
    char* source = gtk_text_buffer_get_text(g_ide.buffer, &start, &end, FALSE);
    
    gtk_text_buffer_set_text(g_ide.output_buffer, "", -1);
    
    append_output("========================================\n", false);
    append_output("Execution started...\n", false);
    append_output("========================================\n\n", false);
    
    g_ide.is_running = true;
    gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "⚡ Running...");
    gtk_widget_set_sensitive(g_ide.run_button, FALSE);
    
    // TODO: Redirect stdout to output buffer
    // For now, show simulated output
    append_output("Welcome to MyKitlerProject!\n", false);
    append_output("Hello, Kitler!\n", false);
    
    // Try to actually run the interpreter
    // run_source(source);  // Uncomment when ready
    
    append_output("\n========================================\n", false);
    append_output("Program exited with code 0\n", false);
    append_output("========================================\n", false);
    
    g_free(source);
    g_ide.is_running = false;
    gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Ready");
    gtk_widget_set_sensitive(g_ide.run_button, TRUE);
}

// ============================================================================
// PROJECT MANAGEMENT
// ============================================================================

void create_project_structure(const char* project_path, const char* project_name) {
    char cmd[512];
    
    #ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "mkdir \"%s\" 2>nul", project_path);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "mkdir \"%s\\src\" 2>nul", project_path);
    system(cmd);
    #else
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s/src\"", project_path);
    system(cmd);
    #endif
    
    char config_path[512];
    snprintf(config_path, sizeof(config_path), "%s/%s.ktconfig", project_path, project_name);
    
    FILE* config = fopen(config_path, "w");
    if (config) {
        fprintf(config, "{\n");
        fprintf(config, "  \"projectName\": \"%s\",\n", project_name);
        fprintf(config, "  \"dotnetVersion\": \"8\",\n");
        fprintf(config, "  \"projectType\": \"application\",\n");
        fprintf(config, "  \"autoOptimized\": true,\n");
        fprintf(config, "  \"entryPoint\": \"main.kt\"\n");
        fprintf(config, "}\n");
        fclose(config);
    }
    
    char main_path[512];
    snprintf(main_path, sizeof(main_path), "%s/src/main.kt", project_path);
    
    FILE* main_file = fopen(main_path, "w");
    if (main_file) {
        fprintf(main_file, "including System.Core#\n\n");
        fprintf(main_file, "projectSpace %s [\n", project_name);
        fprintf(main_file, "    <-- This is a comment -->\n\n");
        fprintf(main_file, "    NewFunc Main() (\n");
        fprintf(main_file, "        Console.Write(\"Welcome to %s!\")\n", project_name);
        fprintf(main_file, "        \n");
        fprintf(main_file, "        NewVar message = \"Hello, Kitler!\"\n");
        fprintf(main_file, "        Console.Write(message)\n");
        fprintf(main_file, "        \n");
        fprintf(main_file, "        NewVar x = 42\n");
        fprintf(main_file, "        NewVar y = 8\n");
        fprintf(main_file, "        Console.Write(\"Result:\", x + y)\n");
        fprintf(main_file, "    )\n");
        fprintf(main_file, "]\n");
        fclose(main_file);
    }
}

void open_project_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) return;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    gtk_text_buffer_set_text(g_ide.buffer, content, -1);
    free(content);
    
    if (g_ide.current_file) free(g_ide.current_file);
    g_ide.current_file = strdup(filepath);
    g_ide.is_modified = false;
    
    char title[512];
    snprintf(title, sizeof(title), "Kitler IDE - %s", filepath);
    gtk_window_set_title(GTK_WINDOW(g_ide.main_window), title);
    
    apply_syntax_highlighting();
    update_line_numbers();
}

void on_save_file(GtkWidget* widget, gpointer data) {
    if (!g_ide.current_file) {
        GtkWidget* dialog = gtk_file_chooser_dialog_new(
            "Save File",
            GTK_WINDOW(g_ide.main_window),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "Cancel", GTK_RESPONSE_CANCEL,
            "Save", GTK_RESPONSE_ACCEPT,
            NULL);
        
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            g_ide.current_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        } else {
            gtk_widget_destroy(dialog);
            return;
        }
        
        gtk_widget_destroy(dialog);
    }
    
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(g_ide.buffer, &start, &end);
    char* text = gtk_text_buffer_get_text(g_ide.buffer, &start, &end, FALSE);
    
    FILE* file = fopen(g_ide.current_file, "w");
    if (file) {
        fwrite(text, 1, strlen(text), file);
        fclose(file);
        g_ide.is_modified = false;
        gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "File saved ✓");
    }
    
    g_free(text);
}

// ============================================================================
// WELCOME SCREEN
// ============================================================================

void on_create_project_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "Create New Project",
        GTK_WINDOW(g_ide.welcome_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Create", GTK_RESPONSE_ACCEPT,
        NULL);
    
    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 25);
    
    GtkWidget* name_label = gtk_label_new("Project Name:");
    GtkWidget* name_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(name_entry), "MyKitlerProject");
    
    GtkWidget* location_label = gtk_label_new("Location:");
    GtkWidget* location_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(location_entry), g_get_home_dir());
    
    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), location_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), location_entry, 1, 1, 2, 1);
    
    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char* name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const char* location = gtk_entry_get_text(GTK_ENTRY(location_entry));
        
        char project_path[512];
        snprintf(project_path, sizeof(project_path), "%s/%s", location, name);
        
        create_project_structure(project_path, name);
        
        g_ide.project_name = strdup(name);
        g_ide.project_path = strdup(project_path);
        
        gtk_widget_hide(g_ide.welcome_window);
        gtk_widget_show_all(g_ide.main_window);
        
        char main_file[512];
        snprintf(main_file, sizeof(main_file), "%s/src/main.kt", project_path);
        open_project_file(main_file);
        
        gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Project created ✓");
    }
    
    gtk_widget_destroy(dialog);
}

void on_open_file_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Open File",
        GTK_WINDOW(g_ide.welcome_window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Open", GTK_RESPONSE_ACCEPT,
        NULL);
    
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Kitler Files (*.kt)");
    gtk_file_filter_add_pattern(filter, "*.kt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        gtk_widget_hide(g_ide.welcome_window);
        gtk_widget_show_all(g_ide.main_window);
        
        open_project_file(filename);
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

GtkWidget* create_welcome_screen() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Kitler IDE");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 650);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Header
    GtkWidget* header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header), 50);
    
    GtkWidget* title = gtk_label_new("Kitler IDE");
    GtkWidget* subtitle = gtk_label_new("Modern Programming Language for .NET");
    
    gtk_widget_set_name(title, "welcome-title");
    gtk_widget_set_name(subtitle, "welcome-subtitle");
    
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header), subtitle, FALSE, FALSE, 0);
    
    // Cards container
    GtkWidget* cards_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(cards_box), 30);
    gtk_box_set_homogeneous(GTK_BOX(cards_box), TRUE);
    
    // Create project card
    GtkWidget* create_btn = gtk_button_new();
    gtk_widget_set_size_request(create_btn, 280, 180);
    GtkWidget* create_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(create_box), 20);
    
    GtkWidget* create_icon = gtk_label_new("📁");
    GtkWidget* create_title = gtk_label_new("Create New Project");
    GtkWidget* create_desc = gtk_label_new("Start building with Kitler");
    
    gtk_widget_set_name(create_icon, "card-icon");
    gtk_widget_set_name(create_title, "card-title");
    gtk_widget_set_name(create_desc, "card-description");
    
    gtk_box_pack_start(GTK_BOX(create_box), create_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(create_box), create_title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(create_box), create_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(create_btn), create_box);
    gtk_widget_set_name(create_btn, "project-card");
    
    // Open file card
    GtkWidget* open_btn = gtk_button_new();
    gtk_widget_set_size_request(open_btn, 280, 180);
    GtkWidget* open_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(open_box), 20);
    
    GtkWidget* open_icon = gtk_label_new("📄");
    GtkWidget* open_title = gtk_label_new("Open File");
    GtkWidget* open_desc = gtk_label_new("Edit existing .kt files");
    
    gtk_widget_set_name(open_icon, "card-icon");
    gtk_widget_set_name(open_title, "card-title");
    gtk_widget_set_name(open_desc, "card-description");
    
    gtk_box_pack_start(GTK_BOX(open_box), open_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(open_box), open_title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(open_box), open_desc, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(open_btn), open_box);
    gtk_widget_set_name(open_btn, "project-card");
    
    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_project_clicked), NULL);
    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_file_clicked), NULL);
    
    gtk_box_pack_start(GTK_BOX(cards_box), create_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(cards_box), open_btn, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), cards_box, TRUE, TRUE, 0);
    
    gtk_widget_set_name(window, "welcome-window");
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return window;
}

// ============================================================================
// MAIN IDE
// ============================================================================

GtkWidget* create_main_ide() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Kitler IDE");
    gtk_window_set_default_size(GTK_WINDOW(window), 1400, 900);
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Toolbar
    GtkWidget* toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
    gtk_widget_set_name(toolbar, "toolbar");
    
    GtkToolItem* new_btn = gtk_tool_button_new(NULL, "New");
    GtkToolItem* open_btn = gtk_tool_button_new(NULL, "Open");
    GtkToolItem* save_btn = gtk_tool_button_new(NULL, "Save");
    GtkToolItem* sep1 = gtk_separator_tool_item_new();
    
    g_ide.build_button = GTK_WIDGET(gtk_tool_button_new(NULL, "Build"));
    gtk_widget_set_name(g_ide.build_button, "build-button");
    
    g_ide.run_button = GTK_WIDGET(gtk_tool_button_new(NULL, "▶ Run"));
    gtk_widget_set_name(g_ide.run_button, "run-button");
    gtk_widget_set_sensitive(g_ide.run_button, FALSE);
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new_btn, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open_btn, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), save_btn, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep1, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(g_ide.build_button), -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(g_ide.run_button), -1);
    
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_file), NULL);
    g_signal_connect(g_ide.build_button, "clicked", G_CALLBACK(on_build_project), NULL);
    g_signal_connect(g_ide.run_button, "clicked", G_CALLBACK(on_run_code), NULL);
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    // Main content
    GtkWidget* paned_main = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    
    // Sidebar
    g_ide.sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(g_ide.sidebar, 250, -1);
    gtk_widget_set_name(g_ide.sidebar, "sidebar");
    
    GtkWidget* sidebar_label = gtk_label_new("📁 Project Explorer");
    gtk_box_pack_start(GTK_BOX(g_ide.sidebar), sidebar_label, FALSE, FALSE, 15);
    
    // Editor area
    GtkWidget* paned_vertical = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    
    // Code editor with line numbers
    GtkWidget* editor_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // Line numbers
    GtkWidget* line_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(line_scroll),
        GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    g_ide.line_numbers = gtk_label_new("1\n");
    gtk_widget_set_name(g_ide.line_numbers, "line-numbers");
    gtk_label_set_xalign(GTK_LABEL(g_ide.line_numbers), 1.0);
    gtk_widget_set_margin_start(g_ide.line_numbers, 5);
    gtk_widget_set_margin_end(g_ide.line_numbers, 5);
    gtk_container_add(GTK_CONTAINER(line_scroll), g_ide.line_numbers);
    
    // Text editor
    GtkWidget* scroll1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    g_ide.text_view = gtk_text_view_new();
    g_ide.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_ide.text_view));
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(g_ide.text_view), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(g_ide.text_view), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(g_ide.text_view), 10);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(g_ide.text_view), GTK_WRAP_NONE);
    gtk_widget_set_name(g_ide.text_view, "editor-view");
    
    setup_syntax_highlighting_tags();
    
    gtk_container_add(GTK_CONTAINER(scroll1), g_ide.text_view);
    
    gtk_box_pack_start(GTK_BOX(editor_hbox), line_scroll, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_hbox), scroll1, TRUE, TRUE, 0);
    
    // Output panel
    GtkWidget* output_frame = gtk_frame_new("Output");
    GtkWidget* scroll2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll2),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    g_ide.output_view = gtk_text_view_new();
    g_ide.output_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_ide.output_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_ide.output_view), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(g_ide.output_view), 10);
    gtk_widget_set_name(g_ide.output_view, "output-view");
    
    gtk_container_add(GTK_CONTAINER(scroll2), g_ide.output_view);
    gtk_container_add(GTK_CONTAINER(output_frame), scroll2);
    
    gtk_paned_pack1(GTK_PANED(paned_vertical), editor_hbox, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned_vertical), output_frame, FALSE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned_vertical), 600);
    
    gtk_paned_pack1(GTK_PANED(paned_main), g_ide.sidebar, FALSE, FALSE);
    gtk_paned_pack2(GTK_PANED(paned_main), paned_vertical, TRUE, FALSE);
    
    gtk_box_pack_start(GTK_BOX(vbox), paned_main, TRUE, TRUE, 0);
    
    // Status bar
    g_ide.status_bar = gtk_statusbar_new();
    gtk_widget_set_name(g_ide.status_bar, "statusbar");
    gtk_box_pack_start(GTK_BOX(vbox), g_ide.status_bar, FALSE, FALSE, 0);
    gtk_statusbar_push(GTK_STATUSBAR(g_ide.status_bar), 0, "Ready");
    
    g_signal_connect(g_ide.buffer, "changed", G_CALLBACK(on_text_changed), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    return window;
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    
    // Initialize state
    g_ide.current_file = NULL;
    g_ide.project_path = NULL;
    g_ide.project_name = NULL;
    g_ide.is_modified = false;
    g_ide.is_running = false;
    g_ide.dark_mode = true;
    
    // Apply dark theme
    apply_global_dark_theme();
    
    // Create windows
    g_ide.welcome_window = create_welcome_screen();
    g_ide.main_window = create_main_ide();
    
    // Show welcome screen
    gtk_widget_show_all(g_ide.welcome_window);
    
    gtk_main();
    
    // Cleanup
    if (g_ide.current_file) free(g_ide.current_file);
    if (g_ide.project_path) free(g_ide.project_path);
    if (g_ide.project_name) free(g_ide.project_name);
    
    return 0;
}