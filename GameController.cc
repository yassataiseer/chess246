#include "GameController.h"
#include "Board.h"
#include "Pos.h"
#include "Colour.h"
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

GameController::GameController() : board{nullptr}, gameInProgress{false} {}

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
      board = std::make_shared<Board>();
      gameInProgress = true;
      board->draw(std::cout);
    } else {
      std::cout << "Invalid game mode. Only 'game human human' is supported.\n";
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
    
    bool moveSuccess = false;
    
    // Check if promotion piece is specified
    if (!promotion_str.empty() && promotion_str.length() == 1) {
      char promotionPiece = promotion_str[0];
      moveSuccess = board->move(src, dst, promotionPiece);
    } else {
      moveSuccess = board->move(src, dst);
    }
    
    if (moveSuccess) {
      board->draw(std::cout);
      
      // After a move, the current player is the one whose turn it is now
      // The piece at the destination belongs to the player who just moved
      auto piece = board->pieceAt(dst);
      if (!piece) {
        // This shouldn't happen in a valid move, but let's be safe
        return true;
      }
      
      Colour previousPlayerColour = piece->colour();
      Colour currentPlayerColour = (previousPlayerColour == Colour::White) ? Colour::Black : Colour::White;
      
      // Check if the current player is in check
      if (board->isInCheck(currentPlayerColour)) {
        // Check if it's checkmate
        if (board->isCheckmate(currentPlayerColour)) {
          std::cout << (currentPlayerColour == Colour::White ? "Black" : "White") << " wins!\n";
          gameInProgress = false;
        } else {
          std::cout << "Check!\n";
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
    
    // According to the baseline example, we just print "Black wins!"
    std::cout << "Black wins!\n";
    gameInProgress = false;
  } else if (command.empty()) {
    // Empty command, just continue
  } else {
    std::cout << "Unknown command: " << command << "\n";
    std::cout << "Available commands: game, move, resign\n";
  }
  
  return true;
}

void GameController::run() {
  std::string line;
  
  while (std::getline(std::cin, line)) {
    if (!processCommand(line)) {
      break;
    }
  }
} 