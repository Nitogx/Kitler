# Kitler (KT) Language Makefile
# Compiles the interpreter using GCC

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LDFLAGS = -lm

# Target executable
TARGET = kt
TARGET_WIN = kt.exe

# Source files
SOURCES = main.c lexer.c parser.c interpreter.c memory.c
OBJECTS = $(SOURCES:.c=.o)

# Header files
HEADERS = types.h

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM = Windows
    RM = del /Q
    EXECUTABLE = $(TARGET_WIN)
else
    PLATFORM = $(shell uname -s)
    RM = rm -f
    EXECUTABLE = $(TARGET)
endif

# Default target
all: $(EXECUTABLE)
	@echo "Build complete for $(PLATFORM)!"
	@echo "Run with: ./$(EXECUTABLE)"

# Link object files into executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

# Compile source files into object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	$(RM) $(OBJECTS) $(EXECUTABLE) $(TARGET_WIN)

# Install to system (Linux/Mac)
install: $(EXECUTABLE)
	@echo "Installing to /usr/local/bin..."
	sudo cp $(EXECUTABLE) /usr/local/bin/
	@echo "Installation complete! Run 'kt' from anywhere."

# Uninstall from system
uninstall:
	@echo "Removing from /usr/local/bin..."
	sudo rm -f /usr/local/bin/$(EXECUTABLE)
	@echo "Uninstalled successfully."

# Run tests
test: $(EXECUTABLE)
	@echo "Running basic tests..."
	@echo "NewVar x = 42" | ./$(EXECUTABLE)
	@echo "Console.Write(\"Test passed!\")" | ./$(EXECUTABLE)

# Create example project
example: $(EXECUTABLE)
	./$(EXECUTABLE) new ExampleGame

# Development build with debug symbols
debug: CFLAGS += -DDEBUG -g3
debug: clean $(EXECUTABLE)
	@echo "Debug build complete!"

# Release build with optimizations
release: CFLAGS += -O3 -DNDEBUG
release: clean $(EXECUTABLE)
	@echo "Release build complete!"

# Help target
help:
	@echo "Kitler (KT) Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the interpreter (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install to /usr/local/bin (requires sudo)"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  test      - Run basic tests"
	@echo "  example   - Create example project"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  help      - Show this help message"

.PHONY: all clean install uninstall test example debug release help