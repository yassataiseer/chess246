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
#include <random> // Added for random number generation

GameController::GameController() 
  : board{nullptr}, 
    gameInProgress{false}, 
    gameOver{false}, 
    setupMode{false},
    whiteScore{0.0},
    blackScore{0.0},
    whitePlayerType{PlayerType::Human},
    blackPlayerType{PlayerType::Human},
    whiteComputerLevel{ComputerLevel::Level1},
    blackComputerLevel{ComputerLevel::Level1},
    rng{std::random_device{}()},
    display{nullptr},
    window{0},
    gc{nullptr},
    cellSize{70},
    graphicsActive{false} {
  whiteSquareColor = 0xF0D9B5;
  blackSquareColor = 0xB58863;
  whitePieceColor = 0xFFFFFF;
  blackPieceColor = 0x000000;
  textColor = 0x000000;
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
    
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (letterPattern[row][col] == 1) {
                XFillRectangle(display, window, gc,
                    offsetX + col * pixelSize,
                    offsetY + row * pixelSize,
                    pixelSize,
                    pixelSize);
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
      
      XDrawString(display, window, gc,
                  8 * cellSize + 30,
                  (7 - rank) * cellSize + cellSize/2 + 20,
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
  
  if (command == "setup") {
    if (gameInProgress) {
      std::cout << "Cannot enter setup mode while a game is in progress.\n";
      return true;
    }
    
    // Initialize a new empty board for setup
    board = std::make_shared<Board>();
    board->clearBoard();
    setupMode = true;
    std::cout << "Entering setup mode.\n";
    
    // Check if graphics are already active
    if (!graphicsActive) {
      // Ask if user wants graphics during setup
      std::cout << "Enable graphics for setup? (y/n): ";
      std::string response;
      std::getline(std::cin, response);
      if (response == "y" || response == "Y" || response == "yes" || response == "Yes") {
        graphicsActive = initGraphics();
        if (graphicsActive) {
          std::cout << "Graphics initialized successfully!" << std::endl;
          renderGraphics();
        } else {
          std::cout << "Graphics initialization failed. Running in text-only mode." << std::endl;
        }
      }
    } else {
      // Graphics already active, just update the display
      renderGraphics();
    }
    
    board->draw(std::cout);
    return true;
  }
  else if (command == "game") {
    std::string player1, player2;
    iss >> player1 >> player2;
    
    if ((player1 == "human" || player1 == "computer") && 
        (player2 == "human" || player2 == "computer")) {
      
      if (gameInProgress) {
        std::cout << "A game is already in progress. Please resign or finish the current game before starting a new one.\n";
        return true;
      }
      
      // Set player types
      whitePlayerType = (player1 == "human") ? PlayerType::Human : PlayerType::Computer;
      blackPlayerType = (player2 == "human") ? PlayerType::Human : PlayerType::Computer;
      
      // Parse level parameters
      int whiteLevel = 1;
      int blackLevel = 1;
      std::string levelStr1, levelStr2;
      
      // Read first level parameter
      if (iss >> levelStr1) {
        // Try to parse "level<N>" format
        if (levelStr1.length() > 5 && levelStr1.substr(0, 5) == "level") {
          try {
            int level = std::stoi(levelStr1.substr(5));
            if (whitePlayerType == PlayerType::Computer) {
              whiteLevel = level;
            } else if (blackPlayerType == PlayerType::Computer) {
              blackLevel = level;
            }
          } catch (...) {
            // Invalid format, keep default level
          }
        } 
        // Try to parse direct number format
        else {
          try {
            int level = std::stoi(levelStr1);
            if (whitePlayerType == PlayerType::Computer) {
              whiteLevel = level;
            } else if (blackPlayerType == PlayerType::Computer) {
              blackLevel = level;
            }
          } catch (...) {
            // Invalid format, keep default level
          }
        }
        
        // Read second level parameter (for computer vs computer games)
        if (whitePlayerType == PlayerType::Computer && blackPlayerType == PlayerType::Computer) {
          if (iss >> levelStr2) {
            // Try to parse "level<N>" format
            if (levelStr2.length() > 5 && levelStr2.substr(0, 5) == "level") {
              try {
                blackLevel = std::stoi(levelStr2.substr(5));
              } catch (...) {
                // Invalid format, keep default level
              }
            } 
            // Try to parse direct number format
            else {
              try {
                blackLevel = std::stoi(levelStr2);
              } catch (...) {
                // Invalid format, keep default level
              }
            }
          }
        }
      }
      
      // Set computer levels (clamp between 1 and 4)
      if (whitePlayerType == PlayerType::Computer) {
        whiteLevel = std::max(1, std::min(4, whiteLevel));
        whiteComputerLevel = static_cast<ComputerLevel>(whiteLevel - 1);
        std::cout << "White computer player set to level " << whiteLevel << std::endl;
      }
      
      if (blackPlayerType == PlayerType::Computer) {
        blackLevel = std::max(1, std::min(4, blackLevel));
        blackComputerLevel = static_cast<ComputerLevel>(blackLevel - 1);
        std::cout << "Black computer player set to level " << blackLevel << std::endl;
      }
      
      board = std::make_shared<Board>();
      gameInProgress = true;
      gameOver = false;
      
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
      
      // If the first player is a computer, make its move
      if ((currentPlayer == Colour::White && whitePlayerType == PlayerType::Computer) ||
          (currentPlayer == Colour::Black && blackPlayerType == PlayerType::Computer)) {
        makeComputerMove();
      }
    } else {
      std::cout << "Invalid game mode. Use 'game human human', 'game human computer', 'game computer human', or 'game computer computer'.\n";
      std::cout << "You can also specify computer level with 'level<N>' where N is 1-4, e.g., 'game human computer level2'.\n";
    }
  } else if (command == "castle") {
    if (!gameInProgress) {
      std::cout << "No game in progress. Use 'game human human' to start.\n";
      return true;
    }
    
    // Check if it's a human player's turn
    if (isComputerTurn()) {
      std::cout << "It's the computer's turn. Please wait.\n";
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
          gameOver = true;
          
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
        gameOver = true;
        
        // Close graphics window
        if (graphicsActive) {
          sleep(2); // Give user a moment to see the final position
          closeGraphics();
          graphicsActive = false;
        }
      }
      
      // If the next player is a computer, make its move
      if (gameInProgress && isComputerTurn()) {
        makeComputerMove();
      }
    } else {
      std::cout << "Invalid castling move.\n";
    }
  } else if (command == "move") {
    if (!gameInProgress) {
      std::cout << "No game in progress. Use 'game human human' to start.\n";
      return true;
    }
    
    // Check if it's a human player's turn
    if (isComputerTurn()) {
      std::cout << "It's the computer's turn. Please wait.\n";
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
          std::cout << "Checkmate! " << (currentPlayerColour == Colour::White ? "Black" : "White") << " wins!" << std::endl;
          incrementScore(currentPlayerColour == Colour::White ? Colour::Black : Colour::White);
          gameInProgress = false;
          gameOver = true;
          
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
        gameOver = true;
        
        // Close graphics window
        if (graphicsActive) {
          sleep(2); // Give user a moment to see the final position
          closeGraphics();
          graphicsActive = false;
        }
      }
      
      // If the next player is a computer, make its move
      if (gameInProgress && isComputerTurn()) {
        makeComputerMove();
      }
    } else {
      std::cout << "Invalid move.\n";
    }
  } else if (command == "resign") {
    if (!gameInProgress) {
      std::cout << "No game in progress.\n";
      return true;
    }
    
    // Check if it's a human player's turn
    if (isComputerTurn()) {
      std::cout << "Computer players cannot resign. Use 'game human human' to start a new game.\n";
      return true;
    }
    
    std::cout << (board->getCurrentTurn() == Colour::White ? "White" : "Black") << " resigns. ";
    std::cout << (board->getCurrentTurn() == Colour::White ? "Black" : "White") << " wins!" << std::endl;
    incrementScore(board->getCurrentTurn() == Colour::White ? Colour::Black : Colour::White);
    gameInProgress = false;
    gameOver = true;
    
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
    std::cout << "  game <player1> <player2> [level] - Start a new game\n";
    std::cout << "    where <player> is 'human' or 'computer'\n";
    std::cout << "    and [level] is an optional computer level (1-4)\n";
    std::cout << "    Examples: 'game human human', 'game human computer 2', 'game computer computer 2 4'\n";
    std::cout << "  move <from> <to> [promotion] - Move a piece (e.g., 'move e2 e4')\n";
    std::cout << "  castle kingside/queenside - Castle on king or queen side\n";
    std::cout << "  setup - Enter setup mode to customize the board\n";
    std::cout << "  resign - Forfeit the game\n";
    std::cout << "  draw\n";
    std::cout << "  score - Display current score\n";
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

bool GameController::processSetupCommand(const std::string& cmd) {
  std::istringstream iss(cmd);
  std::string command;
  iss >> command;
  
  if (command == "+") {
    char pieceType;
    std::string posStr;
    iss >> pieceType >> posStr;
    
    if (pieceType == 0 || posStr.empty()) {
      std::cout << "Invalid command format. Use '+ [piece] [position]'.\n";
      return true;
    }
    
    Pos pos = parsePos(posStr);
    if (pos.file == -1) {
      std::cout << "Invalid position.\n";
      return true;
    }
    
    Colour colour = isupper(pieceType) ? Colour::White : Colour::Black;
    
    board->placePiece(pos, pieceType, colour);
    
    if (graphicsActive) {
      renderGraphics();
    }
    
    board->draw(std::cout);
  } 
  else if (command == "-") {
    std::string posStr;
    iss >> posStr;
    
    if (posStr.empty()) {
      std::cout << "Invalid command format. Use '- [position]'.\n";
      return true;
    }
    
    Pos pos = parsePos(posStr);
    if (pos.file == -1) {
      std::cout << "Invalid position.\n";
      return true;
    }
    
    board->removePiece(pos);
    
    if (graphicsActive) {
      renderGraphics();
    }
    
    board->draw(std::cout);
  } 
  else if (command == "=") {
    std::string colourStr;
    iss >> colourStr;
    
    if (colourStr == "white") {
      board->setCurrentTurn(Colour::White);
      std::cout << "Set white to play next.\n";
    } else if (colourStr == "black") {
      board->setCurrentTurn(Colour::Black);
      std::cout << "Set black to play next.\n";
    } else {
      std::cout << "Invalid colour. Use 'white' or 'black'.\n";
    }
  }
  else if (command == "graphics") {
    if (graphicsActive) {
      std::cout << "Closing graphics window...\n";
      closeGraphics();
      graphicsActive = false;
    } else {
      std::cout << "Opening graphics window...\n";
      graphicsActive = initGraphics();
      if (graphicsActive) {
        std::cout << "Graphics initialized successfully!" << std::endl;
        renderGraphics();
      } else {
        std::cout << "Graphics initialization failed." << std::endl;
      }
    }
    return true;
  }
  else if (command == "done") {
    if (validateBoard()) {
      setupMode = false;
      std::cout << "Exiting setup mode. Board is valid.\n";
      gameInProgress = true;
      gameOver = false;
      
      if (!graphicsActive) {
        graphicsActive = initGraphics();
        
        if (graphicsActive) {
          std::cout << "Graphics initialized successfully!" << std::endl;
        } else {
          std::cout << "Graphics initialization failed. Running in text-only mode." << std::endl;
        }
      }
      
      if (graphicsActive) {
        renderGraphics();
      }
      
      board->draw(std::cout);
      
      int pieceCount = 0;
      bool hasOnlyKings = true;
      
      for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
          auto piece = board->pieceAt({file, rank});
          if (piece) {
            pieceCount++;
            char symbol = piece->symbol();
            if (symbol != 'K' && symbol != 'k') {
              hasOnlyKings = false;
            }
          }
        }
      }
      
      if (pieceCount == 2 && hasOnlyKings) {
        std::cout << "Kings-only position: Stalemate! The game is a draw.\n";
        incrementDrawScore();
        gameInProgress = false;
        gameOver = true;
        
        if (graphicsActive) {
          sleep(2);
          closeGraphics();
          graphicsActive = false;
        }
        return true;
      }
      
      Colour currentPlayerColour = board->getCurrentTurn();
      if (board->isStalemate(currentPlayerColour)) {
        std::cout << "Stalemate! The game is a draw.\n";
        gameInProgress = false;
        gameOver = true;
        
        if (graphicsActive) {
          sleep(2);
          closeGraphics();
          graphicsActive = false;
        }
      } else if (board->isInCheck(currentPlayerColour)) {
        if (board->isCheckmate(currentPlayerColour)) {
          std::cout << (currentPlayerColour == Colour::White ? "Black" : "White") << " wins!\n";
          gameInProgress = false;
          gameOver = true;
          
          if (graphicsActive) {
            sleep(2);
            closeGraphics();
            graphicsActive = false;
          }
        } else {
          std::cout << "Check!\n";
        }
      }
      
      std::cout << "\n" << (currentPlayerColour == Colour::White ? "White" : "Black") << " to play." << std::endl;
    } else {
      std::cout << "Invalid board configuration. Cannot exit setup mode.\n";
    }
  } 
  else {
    std::cout << "Unknown setup command. Available commands:\n";
    std::cout << "+ [piece] [position] - Add a piece (e.g., '+ K e1' for white king, '+ k e8' for black king)\n";
    std::cout << "- [position] - Remove a piece (e.g., '- e2')\n";
    std::cout << "= [color] - Set turn (e.g., '= white' or '= black')\n";
    std::cout << "graphics - Toggle graphics display on/off\n";
    std::cout << "done - Exit setup mode\n";
  }
  
  return true;
}

bool GameController::validateBoard() const {
  bool whiteKingFound = false;
  bool blackKingFound = false;
  int pieceCount = 0;
  
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = board->pieceAt({file, rank});
      if (piece) {
        pieceCount++;
        char symbol = piece->symbol();
        if (symbol == 'K') {
          whiteKingFound = true;
        } else if (symbol == 'k') {
          blackKingFound = true;
        }
      }
    }
  }
  
  if (!whiteKingFound) {
    std::cout << "Invalid board: No white king found.\n";
    return false;
  }
  
  if (!blackKingFound) {
    std::cout << "Invalid board: No black king found.\n";
    return false;
  }
  
  for (int file = 0; file < 8; ++file) {
    auto piece1 = board->pieceAt({file, 0});
    auto piece2 = board->pieceAt({file, 7});
    
    if ((piece1 && (piece1->symbol() == 'P' || piece1->symbol() == 'p')) || 
        (piece2 && (piece2->symbol() == 'P' || piece2->symbol() == 'p'))) {
      std::cout << "Invalid board: Pawn on the first or last rank.\n";
      return false;
    }
  }
  
  if (board->isInCheck(Colour::White)) {
    std::cout << "Invalid board: White king is in check.\n";
    return false;
  }
  
  if (board->isInCheck(Colour::Black)) {
    std::cout << "Invalid board: Black king is in check.\n";
    return false;
  }
  
  if (pieceCount == 2 && whiteKingFound && blackKingFound) {
    std::cout << "Warning: Board has only two kings. This is an immediate stalemate (draw).\n";
    std::cout << "The game will end in a draw after setup is completed.\n";
  }
  
  return true;
}

void GameController::run() {
  std::string line;

  std::cout << "Chess Game\n";
  std::cout << "Enter commands:\n";
  std::cout << "  - game human human - Start a game between two humans\n";
  std::cout << "  - game human computer [level] - Start a game against the computer\n";
  std::cout << "  - game computer human [level] - Let the computer play as white\n";
  std::cout << "  - game computer computer [level] - Watch two computer players\n";
  std::cout << "  - move <from> <to> [promotion] (e.g., 'move e2 e4')\n";
  std::cout << "  - setup (enter setup mode)\n";
  std::cout << "  - resign - Forfeit the game\n";
  std::cout << "  - draw\n";
  std::cout << "  - score - Display current score\n";
  std::cout << "  - quit or exit - Quit the game\n";
  
  bool running = true;
  
  // Print the first prompt
  std::cout << "Enter command: " << std::flush;
  
  while (running) {
    // If it's a computer's turn, make its move
    if (gameInProgress && !gameOver && isComputerTurn()) {
      // Add a small delay between computer moves for better visibility
      if (whitePlayerType == PlayerType::Computer && blackPlayerType == PlayerType::Computer) {
        sleep(2); // 2 second delay in computer vs. computer games
      } else {
        // Add a delay for single computer player moves too
        sleep(1); // 1 second delay for computer vs. human games
      }
      makeComputerMove();
      
      // In computer vs. computer games, continue until game is over or user interrupts
      if (gameInProgress && whitePlayerType == PlayerType::Computer && blackPlayerType == PlayerType::Computer) {
        // Check for user input without blocking
        struct pollfd fds[1];
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        
        if (poll(fds, 1, 0) > 0) {
          // User input available, break out of computer vs. computer loop
          if (!std::getline(std::cin, line)) {
            break; // End of input
          }
          
          if (!processCommand(line)) {
            running = false;
          }
          
          // Print prompt for next command if still running
          if (running) {
            std::cout << "Enter command: " << std::flush;
          }
          continue;
        }
        
        // Continue with computer moves
        continue;
      }
    }
    
    // Check for graphics events if active
    if (graphicsActive) {
      // Process any pending graphics events
      if (!processGraphicsEvents()) {
        // Window was closed
        graphicsActive = false;
        std::cout << "Graphics window closed. Running in text-only mode.\n";
      }
    }
    
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    
    if (poll(fds, 1, 10) > 0) {
      if (!std::getline(std::cin, line)) {
        running = false;
        break;
      }
      
      bool shouldContinue = true;
      if (setupMode) {
        shouldContinue = processSetupCommand(line);
      } else {
        shouldContinue = processCommand(line);
      }
      
      if (!shouldContinue) {
        running = false;
      }
      
      // Print prompt for next command if still running
      if (running) {
        std::cout << "Enter command: " << std::flush;
      }
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

// Check if it's a computer's turn
bool GameController::isComputerTurn() const {
  if (!board || !gameInProgress) return false;
  
  Colour currentTurn = board->getCurrentTurn();
  return (currentTurn == Colour::White && whitePlayerType == PlayerType::Computer) ||
         (currentTurn == Colour::Black && blackPlayerType == PlayerType::Computer);
}

// Make a computer move based on the current difficulty level
void GameController::makeComputerMove() {
  if (!board || !gameInProgress) return;
  
  Colour currentTurn = board->getCurrentTurn();
  ComputerLevel level = (currentTurn == Colour::White) ? whiteComputerLevel : blackComputerLevel;
  
  // Get all legal moves for the current player
  std::vector<Move> legalMoves = getAllLegalMoves(currentTurn);
  
  if (legalMoves.empty()) {
    // No legal moves available - should be checkmate or stalemate
    return;
  }
  
  // Choose a move based on the difficulty level
  Move chosenMove = getRandomMove(legalMoves); // Default to random (Level 1)
  
  switch (level) {
    case ComputerLevel::Level1:
      // Level 1: Random legal moves (already set as default)
      break;
      
    case ComputerLevel::Level2:
      // Level 2: Prefers capturing moves and checks
      chosenMove = getBestMoveLevel2(legalMoves);
      break;
      
    case ComputerLevel::Level3:
      // Level 3: Prefers avoiding capture, capturing moves, and checks
      chosenMove = getBestMoveLevel3(legalMoves);
      break;
      
    case ComputerLevel::Level4:
      // Level 4: More sophisticated strategy with piece values and position evaluation
      chosenMove = getBestMoveLevel4(legalMoves);
      break;
  }
  
  // Make the chosen move
  bool moveSuccess = false;
  if (chosenMove.promotion != '\0') {
    moveSuccess = board->move(chosenMove.from, chosenMove.to, chosenMove.promotion);
  } else {
    moveSuccess = board->move(chosenMove.from, chosenMove.to);
  }
  
  if (moveSuccess) {
    char fromFile = 'a' + chosenMove.from.file;
    char fromRank = '1' + chosenMove.from.rank;
    char toFile = 'a' + chosenMove.to.file;
    char toRank = '1' + chosenMove.to.rank;
    
    std::cout << "Computer moves: " << fromFile << fromRank << " to " << toFile << toRank;
    if (chosenMove.promotion != '\0') {
      std::cout << " (promotion to " << chosenMove.promotion << ")";
    }
    std::cout << std::endl;
    
    if (graphicsActive) {
      renderGraphics();
    }
    
    board->draw(std::cout);
    
    Colour nextPlayer = board->getCurrentTurn();
    std::cout << "\n" << (nextPlayer == Colour::White ? "White" : "Black") << " to play." << std::endl;
    
    // Check for check/checkmate/stalemate
    if (board->isInCheck(nextPlayer)) {
      if (board->isCheckmate(nextPlayer)) {
        Colour winner = (nextPlayer == Colour::White) ? Colour::Black : Colour::White;
        std::cout << "Checkmate! " << (winner == Colour::White ? "White" : "Black") << " wins!" << std::endl;
        incrementScore(winner);
        gameInProgress = false;
        gameOver = true;
        
        // Close graphics window
        if (graphicsActive) {
          sleep(2); // Give user a moment to see the final position
          closeGraphics();
          graphicsActive = false;
        }
      } else {
        std::cout << "Check!" << std::endl;
      }
    } else if (board->isStalemate(nextPlayer)) {
      std::cout << "Stalemate! The game is a draw." << std::endl;
      incrementDrawScore();
      gameInProgress = false;
      gameOver = true;
      
      // Close graphics window
      if (graphicsActive) {
        sleep(2); // Give user a moment to see the final position
        closeGraphics();
        graphicsActive = false;
      }
    }
  }
}

// Get all legal moves for a given color
std::vector<Move> GameController::getAllLegalMoves(Colour colour) const {
  std::vector<Move> legalMoves;
  
  if (!board) return legalMoves;
  
  // Check all pieces of the given color
  for (int srcRank = 0; srcRank < 8; ++srcRank) {
    for (int srcFile = 0; srcFile < 8; ++srcFile) {
      Pos src{srcFile, srcRank};
      auto piece = board->pieceAt(src);
      
      // Skip if no piece or not the player's piece
      if (!piece || piece->colour() != colour) {
        continue;
      }
      
      // Get all legal moves for this piece
      auto pieceMoves = piece->legalMoves(*board, src);
      
      // Check if each move is legal (doesn't leave the king in check)
      for (const auto& dst : pieceMoves) {
        if (board->simulateMove(src, dst, colour)) {
          // Check for pawn promotion
          if ((piece->symbol() == 'P' || piece->symbol() == 'p') && 
              ((colour == Colour::White && dst.rank == 7) || 
               (colour == Colour::Black && dst.rank == 0))) {
            // Add promotion options (Q, R, B, N)
            legalMoves.push_back(Move(src, dst, 'Q'));
            legalMoves.push_back(Move(src, dst, 'R'));
            legalMoves.push_back(Move(src, dst, 'B'));
            legalMoves.push_back(Move(src, dst, 'N'));
          } else {
            legalMoves.push_back(Move(src, dst));
          }
        }
      }
    }
  }
  
  return legalMoves;
}

// Get a random move from the list of legal moves
Move GameController::getRandomMove(const std::vector<Move>& moves) const {
  if (moves.empty()) {
    return Move({0, 0}, {0, 0});
  }
  
  std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
  return moves[dist(rng)];
}

// Level 2: Prefers capturing moves and checks
Move GameController::getBestMoveLevel2(const std::vector<Move>& moves) const {
  if (moves.empty()) {
    return Move({0, 0}, {0, 0});
  }
  
  // Categorize moves
  std::vector<Move> capturingMoves;
  std::vector<Move> checkingMoves;
  std::vector<Move> normalMoves;
  
  for (const auto& move : moves) {
    if (isCapturingMove(move)) {
      capturingMoves.push_back(move);
    } else if (isCheckingMove(move)) {
      checkingMoves.push_back(move);
    } else {
      normalMoves.push_back(move);
    }
  }
  
  if (!capturingMoves.empty()) {
    return getRandomMove(capturingMoves);
  }
  
  if (!checkingMoves.empty()) {
    return getRandomMove(checkingMoves);
  }
  
  return getRandomMove(normalMoves);
}

// Level 3: Prefers avoiding capture, capturing moves, and checks
Move GameController::getBestMoveLevel3(const std::vector<Move>& moves) const {
  if (moves.empty()) {
    return Move({0, 0}, {0, 0});
  }
  
  // Categorize moves with priorities
  std::vector<Move> safeCaptureCheckMoves; // Highest priority: safe moves that capture and check
  std::vector<Move> safeCaptureMoves;      // High priority: safe moves that capture
  std::vector<Move> safeCheckMoves;        // Medium-high priority: safe moves that check
  std::vector<Move> safeMoves;             // Medium priority: moves that don't put the piece in danger
  std::vector<Move> capturingMoves;        // Medium-low priority: capturing moves (even if unsafe)
  std::vector<Move> checkingMoves;         // Low priority: checking moves (even if unsafe)
  std::vector<Move> normalMoves;           // Lowest priority: all other moves
  
  for (const auto& move : moves) {
    bool isCapture = isCapturingMove(move);
    bool isCheck = isCheckingMove(move);
    bool isDangerous = movePutsInDanger(move);
    
    if (!isDangerous && isCapture && isCheck) {
      safeCaptureCheckMoves.push_back(move);
    } else if (!isDangerous && isCapture) {
      safeCaptureMoves.push_back(move);
    } else if (!isDangerous && isCheck) {
      safeCheckMoves.push_back(move);
    } else if (!isDangerous) {
      safeMoves.push_back(move);
    } else if (isCapture) {
      capturingMoves.push_back(move);
    } else if (isCheck) {
      checkingMoves.push_back(move);
    } else {
      normalMoves.push_back(move);
    }
  }
  
  if (!safeCaptureCheckMoves.empty()) return getRandomMove(safeCaptureCheckMoves);
  if (!safeCaptureMoves.empty()) return getRandomMove(safeCaptureMoves);
  if (!safeCheckMoves.empty()) return getRandomMove(safeCheckMoves);
  if (!safeMoves.empty()) return getRandomMove(safeMoves);
  if (!capturingMoves.empty()) return getRandomMove(capturingMoves);
  if (!checkingMoves.empty()) return getRandomMove(checkingMoves);
  
  return getRandomMove(normalMoves);
}

// Level 4
Move GameController::getBestMoveLevel4(const std::vector<Move>& moves) const {
  if (moves.empty()) {
    return Move({0, 0}, {0, 0});
  }
  
  Move bestMove = moves[0];
  int bestScore = -999999; 
  
  Colour currentPlayer = board->getCurrentTurn();
  
  for (const auto& move : moves) {
    // Create a temporary board to simulate the move
    Board tempBoard = *board;
    
    bool moveSuccess = false;
    if (move.promotion != '\0') {
      moveSuccess = tempBoard.move(move.from, move.to, move.promotion);
    } else {
      moveSuccess = tempBoard.move(move.from, move.to);
    }
    
    if (!moveSuccess) continue;
    
    int score = evaluatePosition(tempBoard, currentPlayer);
    
    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }
  
  return bestMove;
}

// Get the value of a piece
int GameController::getPieceValue(char pieceSymbol) const {
  switch (toupper(pieceSymbol)) {
    case 'P': return 100;   // Pawn
    case 'N': return 320;   // Knight
    case 'B': return 330;   // Bishop
    case 'R': return 500;   // Rook
    case 'Q': return 900;   // Queen
    case 'K': return 20000; // King
    default: return 0;
  }
}

// Evaluate a board position from the perspective of the given color
int GameController::evaluatePosition(const Board& board, Colour perspective) const {
  int score = 0;
  
  const int pawnPositionBonus[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5,  5, 10, 25, 25, 10,  5,  5},
    {0,  0,  0, 20, 20,  0,  0,  0},
    {5, -5,-10,  0,  0,-10, -5,  5},
    {5, 10, 10,-20,-20, 10, 10,  5},
    {0,  0,  0,  0,  0,  0,  0,  0}
  };
  
  const int knightPositionBonus[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
  };
  
  const int bishopPositionBonus[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5,  5,  5,  5,  5,-10},
    {-10,  0,  5,  0,  0,  5,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
  };
  
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      Pos pos{file, rank};
      auto piece = board.pieceAt(pos);
      
      if (piece) {
        int pieceValue = getPieceValue(piece->symbol());
        int positionBonus = 0;
        
        char pieceType = toupper(piece->symbol());
        if (pieceType == 'P') {
          positionBonus = pawnPositionBonus[rank][file];
        } else if (pieceType == 'N') {
          positionBonus = knightPositionBonus[rank][file];
        } else if (pieceType == 'B') {
          positionBonus = bishopPositionBonus[rank][file];
        }
        
        if (piece->colour() == Colour::Black) {
          positionBonus = pawnPositionBonus[7 - rank][file];
        }
        
        if (piece->colour() == perspective) {
          score += pieceValue + positionBonus;
        } else {
          score -= pieceValue + positionBonus;
        }
      }
    }
  }
  
  Colour opponent = (perspective == Colour::White) ? Colour::Black : Colour::White;
  if (board.isInCheck(opponent)) {
    score += 50;
  }
  
  if (board.isInCheck(perspective)) {
    score -= 50;
  }
  
  if (board.isCheckmate(opponent)) {
    score += 10000;
  }
  
  if (board.isCheckmate(perspective)) {
    score -= 10000;
  }
  
  return score;
}

// Check if a move is a capturing move
bool GameController::isCapturingMove(const Move& move) const {
  if (!board) return false;
  
  auto destPiece = board->pieceAt(move.to);
  return destPiece != nullptr && destPiece->colour() != board->getCurrentTurn();
}

// Check if a move puts the opponent in check
bool GameController::isCheckingMove(const Move& move) const {
  if (!board) return false;
  
  Board tempBoard = *board;
  
  bool moveSuccess = false;
  if (move.promotion != '\0') {
    moveSuccess = tempBoard.move(move.from, move.to, move.promotion);
  } else {
    moveSuccess = tempBoard.move(move.from, move.to);
  }
  
  if (!moveSuccess) return false;
  
  // Check if the opponent is in check after the move
  Colour opponentColour = (board->getCurrentTurn() == Colour::White) ? Colour::Black : Colour::White;
  return tempBoard.isInCheck(opponentColour);
}

// Check if a move puts the piece in danger (could be captured in the next move)
bool GameController::movePutsInDanger(const Move& move) const {
  if (!board) return false;
  
  // Create a temporary copy of the board to simulate the move
  Board tempBoard = *board;
  
  // Make the move on the temporary board
  bool moveSuccess = false;
  if (move.promotion != '\0') {
    moveSuccess = tempBoard.move(move.from, move.to, move.promotion);
  } else {
    moveSuccess = tempBoard.move(move.from, move.to);
  }
  
  if (!moveSuccess) return false;
  
  Colour opponentColour = (board->getCurrentTurn() == Colour::White) ? Colour::Black : Colour::White;
  return tempBoard.isSquareAttacked(move.to, opponentColour);
} 
