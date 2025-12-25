#ifndef DOTNET_BRIDGE_H
#define DOTNET_BRIDGE_H

#include "types.h"
#include <stdbool.h>

/* Main header for dotnet compatibility - by soso */

/*
 * This header defines the C interface for bridging to .NET/WPF
 * The actual implementation would use .NET hosting APIs or P/Invoke
 * 
 * For production, this would link against a C++/CLI or COM wrapper
 * around the .NET runtime and WPF libraries.
 */

// Forward declarations for .NET bridge types
typedef void* DotNetWindow;
typedef void* DotNetComponent;
typedef void* DotNetGraphics;

// Window types
typedef enum {
    WINDOW_WINDOWED,
    WINDOW_BORDERLESS,
    WINDOW_FULLSCREEN,
    WINDOW_MAXIMIZED,
    WINDOW_MINIMIZED
} WindowType;

// Component types
typedef enum {
    COMPONENT_BUTTON,
    COMPONENT_LABEL,
    COMPONENT_INPUT,
    COMPONENT_PANEL,
    COMPONENT_SLIDER,
    COMPONENT_IMAGE,
    COMPONENT_CANVAS
} ComponentType;

// Color structure
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

// Point/Vector structure
typedef struct {
    double x;
    double y;
} Point;

// Rectangle structure
typedef struct {
    double x;
    double y;
    double width;
    double height;
} Rect;

// Component properties
typedef struct {
    char* text;
    Point position;
    double width;
    double height;
    Color background;
    Color foreground;
    bool visible;
    bool enabled;
    void (*onClick)(void* userData);
    void* userData;
} ComponentProps;

// ============================================================================
// INITIALIZATION & CLEANUP
// ============================================================================

// Initialize .NET runtime and WPF
bool dotnet_init(int dotnet_version);

// Shutdown .NET runtime
void dotnet_shutdown();

// Check if .NET is initialized
bool dotnet_is_initialized();

// ============================================================================
// WINDOW MANAGEMENT
// ============================================================================

// Create main application window
DotNetWindow dotnet_create_window(
    const char* title,
    WindowType type,
    int width,
    int height
);

// Show window
void dotnet_window_show(DotNetWindow window);

// Hide window
void dotnet_window_hide(DotNetWindow window);

// Close window
void dotnet_window_close(DotNetWindow window);

// Set window title
void dotnet_window_set_title(DotNetWindow window, const char* title);

// Set window size
void dotnet_window_set_size(DotNetWindow window, int width, int height);

// Get window size
void dotnet_window_get_size(DotNetWindow window, int* width, int* height);

// Set window position
void dotnet_window_set_position(DotNetWindow window, int x, int y);

// Run message loop (blocking)
void dotnet_run_message_loop();

// Process messages (non-blocking, for game loop)
void dotnet_process_messages();

// ============================================================================
// COMPONENT CREATION
// ============================================================================

// Create button component
DotNetComponent dotnet_create_button(
    DotNetWindow parent,
    const char* text,
    double x, double y,
    double width, double height,
    void (*onClick)(void* userData),
    void* userData
);

// Create label component
DotNetComponent dotnet_create_label(
    DotNetWindow parent,
    const char* text,
    double x, double y,
    int fontSize,
    Color color
);

// Create input field
DotNetComponent dotnet_create_input(
    DotNetWindow parent,
    const char* placeholder,
    double x, double y,
    double width, double height
);

// Create panel (container)
DotNetComponent dotnet_create_panel(
    DotNetWindow parent,
    double x, double y,
    double width, double height,
    Color background
);

// Create slider
DotNetComponent dotnet_create_slider(
    DotNetWindow parent,
    double min, double max, double value,
    double x, double y,
    double width,
    void (*onChange)(double value, void* userData),
    void* userData
);

// Create image component
DotNetComponent dotnet_create_image(
    DotNetWindow parent,
    const char* imagePath,
    double x, double y,
    double width, double height
);

// Create canvas for custom drawing
DotNetComponent dotnet_create_canvas(
    DotNetWindow parent,
    double x, double y,
    double width, double height
);

// ============================================================================
// COMPONENT MANIPULATION
// ============================================================================

// Set component visibility
void dotnet_component_set_visible(DotNetComponent component, bool visible);

// Set component enabled state
void dotnet_component_set_enabled(DotNetComponent component, bool enabled);

// Set component position
void dotnet_component_set_position(DotNetComponent component, double x, double y);

// Set component size
void dotnet_component_set_size(DotNetComponent component, double width, double height);

// Set button text
void dotnet_button_set_text(DotNetComponent button, const char* text);

// Set label text
void dotnet_label_set_text(DotNetComponent label, const char* text);

// Get input text
const char* dotnet_input_get_text(DotNetComponent input);

// Set input text
void dotnet_input_set_text(DotNetComponent input, const char* text);

// Get slider value
double dotnet_slider_get_value(DotNetComponent slider);

// Set slider value
void dotnet_slider_set_value(DotNetComponent slider, double value);

// Remove component
void dotnet_component_remove(DotNetComponent component);

// ============================================================================
// DRAWING OPERATIONS
// ============================================================================

// Get graphics context for canvas
DotNetGraphics dotnet_canvas_get_graphics(DotNetComponent canvas);

// Clear canvas
void dotnet_graphics_clear(DotNetGraphics graphics, Color color);

// Draw rectangle
void dotnet_graphics_draw_rect(
    DotNetGraphics graphics,
    double x, double y,
    double width, double height,
    Color color,
    bool filled
);

// Draw circle
void dotnet_graphics_draw_circle(
    DotNetGraphics graphics,
    double x, double y,
    double radius,
    Color color,
    bool filled
);

// Draw line
void dotnet_graphics_draw_line(
    DotNetGraphics graphics,
    double x1, double y1,
    double x2, double y2,
    Color color,
    double thickness
);

// Draw text
void dotnet_graphics_draw_text(
    DotNetGraphics graphics,
    const char* text,
    double x, double y,
    int fontSize,
    Color color,
    const char* fontFamily
);

// Draw image
void dotnet_graphics_draw_image(
    DotNetGraphics graphics,
    const char* imagePath,
    double x, double y,
    double width, double height
);

// ============================================================================
// INPUT HANDLING
// ============================================================================

// Key codes
typedef enum {
    KEY_UNKNOWN = 0,
    KEY_A = 65, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0 = 48, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_SPACE = 32,
    KEY_ENTER = 13,
    KEY_ESCAPE = 27,
    KEY_LEFT = 37,
    KEY_UP = 38,
    KEY_RIGHT = 39,
    KEY_DOWN = 40,
    KEY_SHIFT = 16,
    KEY_CONTROL = 17,
    KEY_ALT = 18
} KeyCode;

// Mouse buttons
typedef enum {
    MOUSE_LEFT = 0,
    MOUSE_RIGHT = 1,
    MOUSE_MIDDLE = 2
} MouseButton;

// Check if key is currently pressed
bool dotnet_input_is_key_down(KeyCode key);

// Check if key was just pressed this frame
bool dotnet_input_is_key_pressed(KeyCode key);

// Check if key was just released this frame
bool dotnet_input_is_key_released(KeyCode key);

// Check if mouse button is down
bool dotnet_input_is_mouse_button_down(MouseButton button);

// Check if mouse button was just pressed
bool dotnet_input_is_mouse_button_pressed(MouseButton button);

// Get mouse position
Point dotnet_input_get_mouse_position();

// ============================================================================
// AUDIO SYSTEM
// ============================================================================

typedef void* DotNetAudio;

// Load audio file
DotNetAudio dotnet_audio_load(const char* filepath);

// Play audio
void dotnet_audio_play(DotNetAudio audio, bool loop, float volume);

// Play audio once (fire-and-forget)
void dotnet_audio_play_oneshot(DotNetAudio audio, float volume);

// Stop audio
void dotnet_audio_stop(DotNetAudio audio);

// Set audio volume
void dotnet_audio_set_volume(DotNetAudio audio, float volume);

// Check if audio is playing
bool dotnet_audio_is_playing(DotNetAudio audio);

// Unload audio
void dotnet_audio_unload(DotNetAudio audio);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Get delta time (time since last frame)
double dotnet_get_delta_time();

// Get total elapsed time
double dotnet_get_elapsed_time();

// Sleep for milliseconds
void dotnet_sleep(int milliseconds);

// Create color from RGBA
Color dotnet_color_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// Create color from RGB (alpha = 255)
Color dotnet_color_rgb(unsigned char r, unsigned char g, unsigned char b);

// Predefined colors
extern const Color COLOR_RED;
extern const Color COLOR_GREEN;
extern const Color COLOR_BLUE;
extern const Color COLOR_WHITE;
extern const Color COLOR_BLACK;
extern const Color COLOR_YELLOW;
extern const Color COLOR_CYAN;
extern const Color COLOR_MAGENTA;
extern const Color COLOR_GRAY;
extern const Color COLOR_TRANSPARENT;

// ============================================================================
// ERROR HANDLING
// ============================================================================

// Get last .NET error message
const char* dotnet_get_last_error();

// Clear error state
void dotnet_clear_error();

#endif // DOTNET_BRIDGE_H

/*
 * IMPLEMENTATION NOTES:
 * 
 * This bridge would be implemented in C++/CLI or as a COM wrapper around .NET.
 * The actual .NET side would use:
 * 
 * - System.Windows for WPF UI
 * - System.Windows.Controls for UI components
 * - System.Windows.Media for drawing
 * - System.Windows.Input for keyboard/mouse
 * - System.Media or NAudio for audio
 * 
 * Example .NET side (C#):
 * 
 * ```csharp
 * public class KitlerBridge {
 *     private Window mainWindow;
 *     
 *     [DllExport]
 *     public static IntPtr CreateWindow(string title, int type, int w, int h) {
 *         var window = new Window {
 *             Title = title,
 *             Width = w,
 *             Height = h
 *         };
 *         
 *         switch (type) {
 *             case 0: window.WindowStyle = WindowStyle.SingleBorderWindow; break;
 *             case 1: window.WindowStyle = WindowStyle.None; break;
 *             case 2: window.WindowState = WindowState.Maximized; break;
 *         }
 *         
 *         return GCHandle.ToIntPtr(GCHandle.Alloc(window));
 *     }
 * }
 * ```
 */