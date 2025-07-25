#include "GameController.h"
#include "Board.h"
#include "Pos.h"
#include "Colour.h"
#include "Piece.h"
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <poll.h>
#include <unistd.h>

GameController::GameController() : board{nullptr}, gameInProgress{false}, graphicsActive{false} {
    // Initialize color scheme
    whiteSquareColor = 0xF0D9B5;  // Light brown/beige
    blackSquareColor = 0xB58863;  // Dark brown
    whitePieceColor = 0xFFFFFF;   // White
    blackPieceColor = 0x000000;   // Black
    textColor = 0x000000;         // Black text for better visibility
}

GameController::~GameController() {
    if (display) {
        if (gc) XFreeGC(display, gc);
        XCloseDisplay(display);
    }
}

bool GameController::initGraphics() {
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

void GameController::closeGraphics() {
    if (display) {
        XDestroyWindow(display, window);
        XFlush(display);
    }
}

void GameController::drawPieceSprite(int x, int y, char symbol) const {
    if (!display || !gc) return;
    
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

void GameController::renderGraphics() const {
    if (!display || !board) return;
    
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
            auto piece = board->pieceAt({file, rank});
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

bool GameController::processGraphicsEvents() {
    if (!display) return false;
    
    if (XPending(display)) {
        XEvent event;
        XNextEvent(display, &event);
        
        switch (event.type) {
            case Expose:
                // Redraw the window
                renderGraphics();
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

Pos GameController::parsePos(const std::string& pos) {
  if (pos.length() != 2) return {-1, -1};
  
  int file = pos[0] - 'a';
  int rank = pos[1] - '1';
  
  if (file < 0 || file > 7 || rank < 0 || rank > 7) return {-1, -1};
  
  return {file, rank};
}

bool GameController::processCommand(const std::string& cmd) {
  std::istringstream iss(cmd);
  std::string command;
  iss >> command;
  
  if (command == "game") {
    std::string player1, player2;
    iss >> player1 >> player2;
    
    if (player1 == "human" && player2 == "human") {
      if (gameInProgress) {
        std::cout << "A game is already in progress. Please resign or finish the current game before starting a new one.\n";
        return true;
      }
      
      board = std::make_shared<Board>();
      gameInProgress = true;
      
      // Initialize graphics
      graphicsActive = initGraphics();
      
      if (graphicsActive) {
        std::cout << "Graphics initialized successfully!" << std::endl;
        renderGraphics();
      } else {
        std::cout << "Graphics initialization failed. Running in text-only mode." << std::endl;
      }
      
      board->draw(std::cout);
      
      // Show whose turn it is
      Colour currentPlayer = board->getCurrentTurn();
      std::cout << "\n" << (currentPlayer == Colour::White ? "White" : "Black") << " to play." << std::endl;
    } else {
      std::cout << "Invalid game mode. Only 'game human human' is supported.\n";
    }
  } else if (command == "castle") {
    if (!gameInProgress) {
      std::cout << "No game in progress. Use 'game human human' to start.\n";
      return true;
    }
    
    std::string side;
    iss >> side;
    
    bool success = false;
    if (side == "kingside" || side == "k") {
      // Determine king's position based on current player
      int rank = (board->getCurrentTurn() == Colour::White) ? 0 : 7;
      Pos kingPos{4, rank};
      Pos destPos{6, rank};
      success = board->move(kingPos, destPos);
    } else if (side == "queenside" || side == "q") {
      // Determine king's position based on current player
      int rank = (board->getCurrentTurn() == Colour::White) ? 0 : 7;
      Pos kingPos{4, rank};
      Pos destPos{2, rank};
      success = board->move(kingPos, destPos);
    } else {
      std::cout << "Invalid castling command. Use 'castle kingside' or 'castle queenside'.\n";
      return true;
    }
    
    if (success) {
      // Update graphics if active
      if (graphicsActive) {
        renderGraphics();
      }
      
      board->draw(std::cout);
      
      // Check for check/checkmate/stalemate
      Colour currentPlayerColour = board->getCurrentTurn();
      if (board->isInCheck(currentPlayerColour)) {
        if (board->isCheckmate(currentPlayerColour)) {
          std::cout << (currentPlayerColour == Colour::White ? "Black" : "White") << " wins!" << std::endl;
          incrementScore(currentPlayerColour == Colour::White ? Colour::Black : Colour::White);
          gameInProgress = false;
          
          // Close graphics window
          if (graphicsActive) {
            sleep(2); // Give user a moment to see the final position
            closeGraphics();
            graphicsActive = false;
          }
        } else {
          std::cout << "Check!" << std::endl;
        }
      } else if (board->isStalemate(currentPlayerColour)) {
        std::cout << "Stalemate! The game is a draw." << std::endl;
        incrementDrawScore();
        gameInProgress = false;
        
        // Close graphics window
        if (graphicsActive) {
          sleep(2); // Give user a moment to see the final position
          closeGraphics();
          graphicsActive = false;
        }
      }
    } else {
      std::cout << "Invalid castling move.\n";
    }
  } else if (command == "move") {
    if (!gameInProgress) {
      std::cout << "No game in progress. Use 'game human human' to start.\n";
      return true;
    }
    
    std::string src_str, dst_str, promotion_str;
    iss >> src_str >> dst_str >> promotion_str;
    
    Pos src = parsePos(src_str);
    Pos dst = parsePos(dst_str);
    
    if (src.file == -1 || dst.file == -1) {
      std::cout << "Invalid position format. Use algebraic notation (e.g., e2 e4).\n";
      return true;
    }
    
    // Check for castling attempt
    if (board->pieceAt(src) && (board->pieceAt(src)->symbol() == 'K' || board->pieceAt(src)->symbol() == 'k') && abs(dst.file - src.file) == 2) {
      if (board->isInCheck(board->pieceAt(src)->colour())) {
        std::cout << "Warning: You cannot castle while in check!\n";
      }
    }
    
    bool moveSuccess = false;
    
    // Check if promotion piece is specified
    if (!promotion_str.empty() && promotion_str.length() == 1) {
      char promotionPiece = promotion_str[0];
      moveSuccess = board->move(src, dst, promotionPiece);
    } else {
      moveSuccess = board->move(src, dst);
    }
    
    if (moveSuccess) {
      // Update graphics if active
      if (graphicsActive) {
        renderGraphics();
      }
      
      board->draw(std::cout);
      
      // After a move, the current player is the one whose turn it is now
      // The piece at the destination belongs to the player who just moved
      Colour currentPlayerColour = board->getCurrentTurn();
      
      // Show whose turn it is
      std::cout << "\n" << (currentPlayerColour == Colour::White ? "White" : "Black") << " to play." << std::endl;
      
      // Check if the current player is in check
      if (board->isInCheck(currentPlayerColour)) {
        // Check if it's checkmate
        if (board->isCheckmate(currentPlayerColour)) {
          std::cout << (currentPlayerColour == Colour::White ? "Black" : "White") << " wins!" << std::endl;
          incrementScore(currentPlayerColour == Colour::White ? Colour::Black : Colour::White);
          gameInProgress = false;
          
          // Close graphics window
          if (graphicsActive) {
            sleep(2); // Give user a moment to see the final position
            closeGraphics();
            graphicsActive = false;
          }
        } else {
          std::cout << "Check!" << std::endl;
        }
      } else if (board->isStalemate(currentPlayerColour)) {
        std::cout << "Stalemate! The game is a draw." << std::endl;
        incrementDrawScore();
        gameInProgress = false;
        
        // Close graphics window
        if (graphicsActive) {
          sleep(2); // Give user a moment to see the final position
          closeGraphics();
          graphicsActive = false;
        }
      }
    } else {
      std::cout << "Invalid move.\n";
    }
  } else if (command == "resign") {
    if (!gameInProgress) {
      std::cout << "No game in progress.\n";
      return true;
    }
    
    std::cout << (board->getCurrentTurn() == Colour::White ? "White" : "Black") << " resigns. ";
    std::cout << (board->getCurrentTurn() == Colour::White ? "Black" : "White") << " wins!" << std::endl;
    incrementScore(board->getCurrentTurn() == Colour::White ? Colour::Black : Colour::White);
    gameInProgress = false;
    
    // Close graphics window
    if (graphicsActive) {
      sleep(1); // Give user a moment to see the final position
      closeGraphics();
      graphicsActive = false;
    }
  } else if (command == "draw") {
    if (!gameInProgress) {
      std::cout << "No active game. Start with 'game human human'.\n";
    } else {
      board->draw(std::cout);
    }
  } else if (command == "score") {
    printScore();
    return true;
  } else if (command == "help") {
    std::cout << "Commands:\n";
    std::cout << "  game human human - Start a new game\n";
    std::cout << "  move <from> <to> [promotion] - Move a piece (e.g., 'move e2 e4')\n";
    std::cout << "  castle kingside/queenside - Castle on king or queen side\n";
    std::cout << "  resign - Forfeit the game\n";
    std::cout << "  draw - Show the current board state\n";
    std::cout << "  score - Display the current score\n";
    std::cout << "  help - Show this help message\n";
    std::cout << "  quit/exit - Exit the game\n";
  } else if (command == "quit" || command == "exit") {
    return false;
  } else if (command.empty()) {
    // Empty command, just continue
  } else {
    std::cout << "Unknown command: " << command << "\n";
    std::cout << "Type 'help' for a list of commands.\n";
  }
  
  return true;
}

void GameController::run() {
  std::string line;
  
  std::cout << "Chess Game\n";
  std::cout << "Enter commands:\n";
  std::cout << "  - game human human (start a new game)\n";
  std::cout << "  - move <from> <to> [promotion] (e.g., 'move e2 e4')\n";
  std::cout << "  - resign (give up the game)\n";
  std::cout << "  - draw (show the board)\n";
  std::cout << "  - score (display current score)\n";
  std::cout << "  - quit or exit (quit the game)\n";
  
  bool running = true;
  while (running) {
    std::cout << "\nEnter command: ";
    
    // Check for graphics events if active
    if (graphicsActive) {
      // Set up polling for input while checking for graphics events
      struct pollfd fds[1];
      fds[0].fd = STDIN_FILENO;
      fds[0].events = POLLIN;
      
      while (true) {
        // Poll with a short timeout to check for graphics events without blocking
        if (poll(fds, 1, 10) > 0) {
          // Input available, break out of polling loop
          break;
        }
        
        // Process graphics events
        if (!processGraphicsEvents()) {
          // Window was closed
          graphicsActive = false;
          std::cout << "Graphics window closed. Running in text-only mode.\n";
          break;
        }
      }
    }
    
    if (!std::getline(std::cin, line)) {
      break; // End of input
    }
    
    if (!processCommand(line)) {
      running = false;
    }
  }
  
  // Close any open graphics window before exiting
  if (graphicsActive) {
    closeGraphics();
  }
  
  printFinalScore();
} 

void GameController::incrementScore(Colour winner) {
    if (winner == Colour::White) {
        whiteScore += 1.0;
    } else if (winner == Colour::Black) {
        blackScore += 1.0;
    }
}

void GameController::incrementDrawScore() {
    whiteScore += 0.5;
    blackScore += 0.5;
}

void GameController::printScore() const {
    std::cout << "Score:" << std::endl;
    std::cout << "White: " << whiteScore << std::endl;
    std::cout << "Black: " << blackScore << std::endl;
}

void GameController::printFinalScore() const {
    std::cout << "Final Score:" << std::endl;
    std::cout << "White: " << whiteScore << std::endl;
    std::cout << "Black: " << blackScore << std::endl;
} 