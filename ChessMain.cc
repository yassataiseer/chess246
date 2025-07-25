#include "Board.h"
#include "Pos.h"
#include "ChessGraphics.h"
#include <iostream>
#include <string>
#include <sstream>
#include <poll.h>
#include <unistd.h>
#include <memory>
#include <X11/Xlib.h>

// Utility function to convert algebraic notation to Pos
Pos parsePos(const std::string& notation) {
  if (notation.length() < 2) return {-1, -1}; // Invalid position
  
  int file = notation[0] - 'a';
  int rank = notation[1] - '1';
  
  if (file < 0 || file > 7 || rank < 0 || rank > 7) {
    return {-1, -1}; // Invalid position
  }
  
  return {file, rank};
}

// Handle X11 events
void handleXEvents(ChessGraphics& graphics) {
  if (graphics.display) {
    while (XPending(graphics.display)) {
      XEvent event;
      XNextEvent(graphics.display, &event);
      
      switch (event.type) {
        case Expose:
          // Window needs to be redrawn
          graphics.render();
          break;
        case ButtonPress:
          // Mouse button was pressed
          break;
      }
    }
  }
}

int main() {
  std::string command;
  std::unique_ptr<Board> board;
  std::unique_ptr<ChessGraphics> graphics;
  bool graphicsActive = false;
  bool gameActive = false;
  bool gameOver = false;
  
  std::cout << "Chess Game\n";
  std::cout << "Enter commands:\n";
  std::cout << "  - game human human (start a new game)\n";
  std::cout << "  - move <from> <to> [promotion] (e.g., 'move e2 e4')\n";
  std::cout << "  - resign (give up the game)\n";
  std::cout << "  - draw (show the board)\n";
  std::cout << "  - quit or exit (quit the game)\n";
  
  bool running = true;
  while (running) {
    std::cout << "\nEnter command: ";
    std::getline(std::cin, command);
    
    if (command == "quit" || command == "exit") {
      running = false;
    } 
    else if (gameActive && !gameOver && command.substr(0, 4) == "game") {
        if (gameActive && !gameOver) {
            std::cout << "A game is already in progress. Please resign or finish the current game before starting a new one.\n";
            continue;
          }
    }
    
    else if (command == "game human human") {
      // Start a new game
      board = std::make_unique<Board>();
      gameActive = true;
      gameOver = false;
      
      // Initialize graphics
      graphics = std::make_unique<ChessGraphics>(*board);
      graphicsActive = graphics->init();
      
      if (graphicsActive) {
        std::cout << "Graphics initialized successfully!" << std::endl;
        graphics->render();
      } else {
        std::cout << "Graphics initialization failed. Running in text-only mode." << std::endl;
      }
      
      // Show initial board
      board->draw(std::cout);
      
      // Show whose turn it is
      Colour currentPlayer = board->getCurrentTurn();
      std::cout << "\n" << (currentPlayer == Colour::White ? "White" : "Black") << " to play." << std::endl;
    } else if (command == "draw") {
      if (!gameActive) {
        std::cout << "No active game. Start with 'game human human'.\n";
      } else {
        board->draw(std::cout);
      }
    } else if (command == "resign") {
      if (!gameActive || gameOver) {
        std::cout << "No active game. Start with 'game human human'.\n";
      } else {
        std::cout << (board->getCurrentTurn() == Colour::White ? "White" : "Black") << " resigns. ";
        std::cout << (board->getCurrentTurn() == Colour::White ? "Black" : "White") << " wins!" << std::endl;
        gameOver = true;
        
        // Close the graphics window after a short delay
        if (graphicsActive && graphics) {
          sleep(1); // Give user a moment to see the final position
          graphics->closeWindow();
          graphicsActive = false;
        }
      }
    } else if (command.substr(0, 4) == "move") {
      if (!gameActive || gameOver) {
        std::cout << "No active game. Start with 'game human human'.\n";
        continue;
      }
      
      std::istringstream iss(command);
      std::string cmd, src_str, dst_str, promotion;
      
      iss >> cmd >> src_str >> dst_str;
      
      // Check for optional promotion piece
      char promotionPiece = 'Q'; // Default to Queen
      if (iss >> promotion && promotion.length() == 1) {
        promotionPiece = promotion[0];
      }
      
      Pos src = parsePos(src_str);
      Pos dst = parsePos(dst_str);
      
      if (src.file == -1 || dst.file == -1) {
        std::cout << "Invalid position format. Use algebraic notation (e.g., e2).\n";
        continue;
      }
      
      // Check for castling attempt
      if (board->pieceAt(src) && (board->pieceAt(src)->symbol() == 'K' || board->pieceAt(src)->symbol() == 'k') && abs(dst.file - src.file) == 2) {
        if (board->isInCheck(board->pieceAt(src)->colour())) {
          std::cout << "Warning: You cannot castle while in check!\n";
        }
      }
      
      bool moveSuccess = board->move(src, dst, promotionPiece);
      if (!moveSuccess) {
        std::cout << "Invalid move\n";
      } else {
        std::cout << "Move successful\n";
        
        // Update the graphics after a successful move
        if (graphicsActive) {
          graphics->render();
        }
        
        // Show text board
        board->draw(std::cout);
        
        // Show whose turn it is
        Colour nextPlayer = board->getCurrentTurn();
        std::cout << "\n" << (nextPlayer == Colour::White ? "White" : "Black") << " to play." << std::endl;
        
        // Check for check/checkmate
        if (board->isInCheck(nextPlayer)) {
          if (board->isCheckmate(nextPlayer)) {
            Colour winner = (nextPlayer == Colour::White) ? Colour::Black : Colour::White;
            std::cout << "Checkmate! " << (winner == Colour::White ? "White" : "Black") << " wins!" << std::endl;
            gameOver = true;
            
            // Close the graphics window after a short delay
            if (graphicsActive && graphics) {
              sleep(2); // Give user a moment to see the checkmate position
              graphics->closeWindow();
              graphicsActive = false;
            }
          } else {
            std::cout << "Check!" << std::endl;
          }
        }
      }
    } else if (command == "help") {
      std::cout << "Commands:\n";
      std::cout << "  game human human - Start a new game\n";
      std::cout << "  move <from> <to> [promotion] - Move a piece (e.g., 'move e2 e4')\n";
      std::cout << "  resign - Forfeit the game\n";
      std::cout << "  draw - Show the current board state\n";
      std::cout << "  help - Show this help message\n";
      std::cout << "  quit/exit - Exit the game\n";
    } else {
      std::cout << "Unknown command. Type 'help' for a list of commands.\n";
    }
    
    // Handle graphics events if active
    if (graphicsActive && graphics) {
      // Set up polling for graphics events
      struct pollfd fds[1];
      fds[0].fd = STDIN_FILENO;
      fds[0].events = POLLIN;
      
      // Poll with a short timeout to check for graphics events without blocking
      if (poll(fds, 1, 10) == 0) { // No input available
        if (!graphics->processEvents()) {
          // Window was closed
          graphicsActive = false;
          std::cout << "Graphics window closed. Running in text-only mode.\n";
        }
      }
    }
  }
  
  // Close any open graphics window before exiting
  if (graphicsActive && graphics) {
    graphics->closeWindow();
  }
  
  std::cout << "Game ended." << std::endl;
  return 0;
} 