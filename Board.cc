#include "Board.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Pawn.h"
#include "Knight.h"
#include "Bishop.h"
#include "Rook.h"
#include "Queen.h"
#include "King.h"
#include <vector>
#include <memory>
#include <ostream>
#include <algorithm>
#include <iostream> // Added for debug output

Board::Board() : grid(8, std::vector<std::shared_ptr<Piece>>(8, nullptr)), currentTurn{Colour::White} {
  // Initialize the board with pieces in their starting positions
  
  // Set up pawns
  for (int file = 0; file < 8; ++file) {
    grid[1][file] = std::make_shared<Pawn>(Colour::White);
    grid[6][file] = std::make_shared<Pawn>(Colour::Black);
  }
  
  // Set up other pieces for White
  grid[0][0] = std::make_shared<Rook>(Colour::White);
  grid[0][1] = std::make_shared<Knight>(Colour::White);
  grid[0][2] = std::make_shared<Bishop>(Colour::White);
  grid[0][3] = std::make_shared<Queen>(Colour::White);
  grid[0][4] = std::make_shared<King>(Colour::White);
  grid[0][5] = std::make_shared<Bishop>(Colour::White);
  grid[0][6] = std::make_shared<Knight>(Colour::White);
  grid[0][7] = std::make_shared<Rook>(Colour::White);
  
  // Set up other pieces for Black
  grid[7][0] = std::make_shared<Rook>(Colour::Black);
  grid[7][1] = std::make_shared<Knight>(Colour::Black);
  grid[7][2] = std::make_shared<Bishop>(Colour::Black);
  grid[7][3] = std::make_shared<Queen>(Colour::Black);
  grid[7][4] = std::make_shared<King>(Colour::Black);
  grid[7][5] = std::make_shared<Bishop>(Colour::Black);
  grid[7][6] = std::make_shared<Knight>(Colour::Black);
  grid[7][7] = std::make_shared<Rook>(Colour::Black);
}

bool Board::isValidPos(Pos p) const {
  return p.file >= 0 && p.file < 8 && p.rank >= 0 && p.rank < 8;
}

std::shared_ptr<Piece> Board::pieceAt(Pos p) const {
  if (!isValidPos(p)) return nullptr;
  return grid[p.rank][p.file];
}

bool Board::simulateMove(Pos src, Pos dst, Colour playerColour) const {
  // Create a new board with the same setup
  Board tempBoard;
  tempBoard.grid = grid;  // Copy the grid
  tempBoard.currentTurn = currentTurn;  // Copy the current turn
  
  // Get the piece at the source position
  auto piece = tempBoard.pieceAt(src);
  if (!piece) return false;
  
  // Special case for pawn promotion (auto-promote to Queen)
  bool isPawnPromotion = false;
  if (auto pawn = dynamic_cast<Pawn*>(piece.get())) {
    if ((piece->colour() == Colour::White && dst.rank == 7) ||
        (piece->colour() == Colour::Black && dst.rank == 0)) {
      isPawnPromotion = true;
    }
  }
  
  // Make the move on the temporary board
  tempBoard.grid[dst.rank][dst.file] = isPawnPromotion ? 
    std::make_shared<Queen>(piece->colour()) : piece;
  tempBoard.grid[src.rank][src.file] = nullptr;
  
  // Check if the player is in check after the move
  return !tempBoard.isInCheck(playerColour);
}

bool Board::isCheckmate(Colour c) const {
  // First check if the player is in check
  if (!isInCheck(c)) {
    return false;
  }
  
  // Try all possible moves for all pieces of the player
  for (int srcRank = 0; srcRank < 8; ++srcRank) {
    for (int srcFile = 0; srcFile < 8; ++srcFile) {
      Pos src{srcFile, srcRank};
      auto piece = pieceAt(src);
      
      // Skip if no piece or not the player's piece
      if (!piece || piece->colour() != c) {
        continue;
      }
      
      // Get all legal moves for this piece
      auto legalMoves = piece->legalMoves(*this, src);
      
      // Try each move to see if it gets out of check
      for (const auto& dst : legalMoves) {
        if (simulateMove(src, dst, c)) {
          // Found a move that gets out of check
          return false;
        }
      }
    }
  }
  
  // No move can get the player out of check
  return true;
}

bool Board::move(Pos src, Pos dst) {
  // Check if source position is valid and has a piece
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if it's the correct player's turn
  if (piece->colour() != currentTurn) return false;
  
  // Get legal moves for the piece
  auto legalMoves = piece->legalMoves(*this, src);
  
  // Check if the destination is a legal move
  if (std::find_if(legalMoves.begin(), legalMoves.end(),
                  [dst](const Pos& p) { return p.file == dst.file && p.rank == dst.rank; }) 
      == legalMoves.end()) {
    return false;
  }
  
  // Check if the move would leave the player in check
  if (!simulateMove(src, dst, currentTurn)) {
    return false;
  }
  
  // Special case for pawn promotion (auto-promote to Queen)
  bool isPawnPromotion = false;
  if (auto pawn = dynamic_cast<Pawn*>(piece.get())) {
    if ((piece->colour() == Colour::White && dst.rank == 7) ||
        (piece->colour() == Colour::Black && dst.rank == 0)) {
      isPawnPromotion = true;
    }
  }
  
  // Make the move
  grid[dst.rank][dst.file] = isPawnPromotion ? 
    std::make_shared<Queen>(piece->colour()) : piece;
  grid[src.rank][src.file] = nullptr;
  
  // Switch turns
  currentTurn = (currentTurn == Colour::White) ? Colour::Black : Colour::White;
  
  return true;
}

bool Board::isInCheck(Colour c) const {
  // Find the king's position
  Pos kingPos{-1, -1};
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece && piece->colour() == c && 
          dynamic_cast<King*>(piece.get())) {
        kingPos = {file, rank};
        break;
      }
    }
    if (kingPos.file != -1) break;
  }
  
  // Check if any opponent piece can capture the king
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece && piece->colour() != c) {
        auto moves = piece->legalMoves(*this, {file, rank});
        if (std::find_if(moves.begin(), moves.end(),
                        [kingPos](const Pos& p) { return p.file == kingPos.file && p.rank == kingPos.rank; }) 
            != moves.end()) {
          return true;
        }
      }
    }
  }
  
  return false;
}

void Board::draw(std::ostream& os) const {
  for (int rank = 7; rank >= 0; --rank) {
    os << rank + 1 << " ";
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece) {
        os << piece->symbol();
      } else {
        os << "_";
      }
      os << " ";
    }
    os << "\n";
  }
  os << "  a b c d e f g h\n";
} 