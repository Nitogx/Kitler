#!/bin/bash
# Build script for Kitler IDE

echo "=========================================="
echo "Building Kitler IDE..."
echo "=========================================="

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if GTK3 is available
if ! pkg-config --exists gtk+-3.0; then
    echo -e "${RED}Error: GTK3 not found!${NC}"
    echo "Please install GTK3 development libraries:"
    echo "  - On MSYS2: pacman -S mingw-w64-ucrt-x86_64-gtk3"
    echo "  - On Ubuntu/Debian: sudo apt-get install libgtk-3-dev"
    echo "  - On Fedora: sudo dnf install gtk3-devel"
    exit 1
fi

echo -e "${YELLOW}Step 1/3: Compiling helper modules...${NC}"

# Compile types.c (memory management)
gcc -c types.c -o types.o `pkg-config --cflags gtk+-3.0` -I.
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to compile types.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ types.o${NC}"

# Compile lexer.c
gcc -c lexer.c -o lexer.o `pkg-config --cflags gtk+-3.0` -I.
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to compile lexer.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ lexer.o${NC}"

# Compile parser.c
gcc -c parser.c -o parser.o `pkg-config --cflags gtk+-3.0` -I.
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to compile parser.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ parser.o${NC}"

# Compile interpreter.c
gcc -c interpreter.c -o interpreter.o `pkg-config --cflags gtk+-3.0` -I.
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to compile interpreter.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ interpreter.o${NC}"

echo ""
echo -e "${YELLOW}Step 2/3: Compiling GUI editor...${NC}"

# Compile GUI editor
gcc -c gui_editor.c -o gui_editor.o `pkg-config --cflags gtk+-3.0` -I.
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to compile gui_editor.c${NC}"
    exit 1
fi
echo -e "${GREEN}✓ gui_editor.o${NC}"

echo ""
echo -e "${YELLOW}Step 3/3: Linking executable...${NC}"

# Link everything together
gcc gui_editor.o types.o lexer.o parser.o interpreter.o \
    -o kitler-ide `pkg-config --libs gtk+-3.0` -lm
    
if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to link executable${NC}"
    exit 1
fi

echo -e "${GREEN}✓ kitler-ide${NC}"

# Clean up object files
echo ""
echo -e "${YELLOW}Cleaning up...${NC}"
rm -f *.o
echo -e "${GREEN}✓ Removed temporary files${NC}"

echo ""
echo "=========================================="
echo -e "${GREEN}Build completed successfully!${NC}"
echo "=========================================="
echo ""
echo "Run with: ./kitler-ide"
echo ""

# Make executable if on Unix-like system
chmod +x kitler-ide 2>/dev/null

exit 0