#include "ChessGraphics.h"
#include "Board.h"
#include "Piece.h"
#include "Pos.h"
#include <iostream>

// Constructor
ChessGraphics::ChessGraphics(const Board& b, int size)
    : board(b), cellSize(size), display(nullptr) {
    
    // Brown color scheme
    whiteSquareColor = 0xF0D9B5;  // Light brown/beige
    blackSquareColor = 0xB58863;  // Dark brown
    whitePieceColor = 0xFFFFFF;   // White
    blackPieceColor = 0x000000;   // Black
    textColor = 0x000000;         // Black text for better visibility
}

// Destructor
ChessGraphics::~ChessGraphics() {
    if (display) {
        XFreeGC(display, gc);
        XCloseDisplay(display);
    }
}

// Initialize X11 graphics
bool ChessGraphics::init() {
    // Open display
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Cannot open display" << std::endl;
        return false;
    }
    
    // Get default screen
    int screen = DefaultScreen(display);
    
    // Calculate window size with padding
    int boardSize = cellSize * 8;
    int windowWidth = boardSize + 40;   // Add padding for coordinates
    int windowHeight = boardSize + 40;  // Add padding for coordinates
    
    // Create window
    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,                      // position
        windowWidth, windowHeight,     // size with padding
        1,                             // border width
        BlackPixel(display, screen),   // border color
        0xD2B48C                       // background color (tan)
    );
    
    // Set window title
    XStoreName(display, window, "Chess");
    
    // Set window size hints to prevent resizing
    XSizeHints sizeHints;
    sizeHints.flags = PMinSize | PMaxSize;
    sizeHints.min_width = windowWidth;
    sizeHints.min_height = windowHeight;
    sizeHints.max_width = windowWidth;
    sizeHints.max_height = windowHeight;
    XSetWMNormalHints(display, window, &sizeHints);
    
    // Select input events
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
    
    // Create graphics context
    gc = XCreateGC(display, window, 0, nullptr);
    
    // Map window
    XMapWindow(display, window);
    
    // Force exposure event
    XFlush(display);
    
    return true;
}

// Draw a pixelated letter piece
void ChessGraphics::drawPieceSprite(int x, int y, char symbol) const {
    // Set appropriate color for the piece
    bool isWhitePiece = (symbol >= 'A' && symbol <= 'Z');
    XSetForeground(display, gc, isWhitePiece ? whitePieceColor : blackPieceColor);
    
    // Always use uppercase for the pattern
    char upperSymbol = toupper(symbol);
    
    // Size of each pixel block
    int pixelSize = cellSize / 10;
    
    // Center in square - these are 5x7 pixel letters
    int offsetX = x + (cellSize - pixelSize * 5) / 2;
    int offsetY = y + (cellSize - pixelSize * 7) / 2;
    
    // Define simple pixelated letters (1 = pixel, 0 = empty)
    // Classic 5x7 pixel font patterns
    const int P[7][5] = {
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,0},
        {1,0,0,0,0},
        {1,0,0,0,0},
        {1,0,0,0,0}
    };
    
    const int N[7][5] = {
        {1,0,0,0,1},
        {1,1,0,0,1},
        {1,0,1,0,1},
        {1,0,0,1,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1}
    };
    
    const int B[7][5] = {
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,0}
    };
    
    const int R[7][5] = {
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,0},
        {1,0,1,0,0},
        {1,0,0,1,0},
        {1,0,0,0,1}
    };
    
    const int Q[7][5] = {
        {0,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,1,0,1},
        {1,0,0,1,0},
        {0,1,1,0,1}
    };
    
    const int K[7][5] = {
        {1,0,0,0,1},
        {1,0,0,1,0},
        {1,0,1,0,0},
        {1,1,0,0,0},
        {1,0,1,0,0},
        {1,0,0,1,0},
        {1,0,0,0,1}
    };
    
    // Choose the correct letter pattern
    const int (*letterPattern)[5] = nullptr;
    switch (upperSymbol) {
        case 'P': letterPattern = P; break;
        case 'N': letterPattern = N; break;
        case 'B': letterPattern = B; break;
        case 'R': letterPattern = R; break;
        case 'Q': letterPattern = Q; break;
        case 'K': letterPattern = K; break;
        default: return; // Invalid symbol
    }
    
    // Draw the pixelated letter - make pixels touch each other for a continuous look
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (letterPattern[row][col] == 1) {
                XFillRectangle(display, window, gc,
                    offsetX + col * pixelSize,
                    offsetY + row * pixelSize,
                    pixelSize,  // Full pixel size with no gaps
                    pixelSize); // Full pixel size with no gaps
            }
        }
    }
}

// Render the chess board
void ChessGraphics::render() const {
    if (!display) return;
    
    // Draw board squares
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            // Calculate position with offset for coordinates
            int x = file * cellSize + 20;  // Offset for rank numbers on left
            int y = (7 - rank) * cellSize + 20;  // Offset for file letters on bottom
            
            // Set square color
            unsigned long squareColor = ((rank + file) % 2 == 0) ? whiteSquareColor : blackSquareColor;
            XSetForeground(display, gc, squareColor);
            
            // Draw square
            XFillRectangle(display, window, gc, x, y, cellSize, cellSize);
            
            // Draw piece if present
            auto piece = board.pieceAt({file, rank});
            if (piece) {
                // Get piece symbol
                char symbol = piece->symbol();
                
                // Draw the piece sprite
                drawPieceSprite(x, y, symbol);
            }
        }
    }
    
    // Draw file labels (a-h)
    XSetForeground(display, gc, textColor);
    for (int file = 0; file < 8; ++file) {
        char label = 'a' + file;
        
        // Draw at bottom
        XDrawString(display, window, gc,
                   file * cellSize + cellSize/2 + 20,  // Add offset
                   8 * cellSize + 35,  // Below the board
                   &label, 1);
    }
    
    // Draw rank labels (1-8)
    for (int rank = 0; rank < 8; ++rank) {
        char label = '1' + rank;
        
        // Draw at left side
        XDrawString(display, window, gc,
                   10,  // Left of the board
                   (7 - rank) * cellSize + cellSize/2 + 20,  // Add offset
                   &label, 1);
        
        // Draw at right side as well
        XDrawString(display, window, gc,
                   8 * cellSize + 30,  // Right of the board
                   (7 - rank) * cellSize + cellSize/2 + 20,  // Add offset
                   &label, 1);
    }
    
    // Flush all pending operations
    XFlush(display);
}

// Process X11 events
bool ChessGraphics::processEvents() {
    if (!display) return false;
    
    if (XPending(display)) {
        XEvent event;
        XNextEvent(display, &event);
        
        switch (event.type) {
            case Expose:
                // Redraw the window
                render();
                break;
            
            case ButtonPress:
                // Handle mouse clicks (for future implementation)
                break;
                
            case KeyPress:
                // Close on Escape key
                {
                    KeySym key;
                    char text[10];
                    XLookupString(&event.xkey, text, 10, &key, 0);
                    if (key == XK_Escape) {
                        return false;
                    }
                }
                break;
        }
    }
    
    return true;
} 

// Method to close the window
void ChessGraphics::closeWindow() {
    if (display) {
        XDestroyWindow(display, window);
        XFlush(display);
    }
} 