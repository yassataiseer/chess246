#ifndef CHESS_GRAPHICS_H
#define CHESS_GRAPHICS_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>

// Forward declaration
class Board;

class ChessGraphics {
private:
    GC gc;
    int cellSize;
    const Board& board;
    
    // Colors
    unsigned long whiteSquareColor;
    unsigned long blackSquareColor;
    unsigned long whitePieceColor;
    unsigned long blackPieceColor;
    unsigned long textColor;
    
    // Draw a pixelated piece
    void drawPieceSprite(int x, int y, char symbol) const;

public:
    // Made public for access from ChessMain.cc
    Display* display;
    Window window;

    ChessGraphics(const Board& b, int size = 70); // Increased default size
    ~ChessGraphics();
    
    // Initialize X11 graphics
    bool init();
    
    // Render the board
    void render() const;
    
    // Process X11 events (returns false when window is closed)
    bool processEvents();
    
    // Method to close the window
    void closeWindow();
};

#endif // CHESS_GRAPHICS_H 