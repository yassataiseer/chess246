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
  // Create a temporary copy of the grid
  auto tempGrid = grid;
  
  // Get the piece at the source position
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Make the move on the temporary grid
  tempGrid[dst.rank][dst.file] = piece;
  tempGrid[src.rank][src.file] = nullptr;
  
  // For en passant, also remove the captured pawn
  if (piece->symbol() == 'P' || piece->symbol() == 'p') {
    if (abs(dst.file - src.file) == 1 && abs(dst.rank - src.rank) == 1 && !pieceAt(dst)) {
      // This is an en passant capture
      tempGrid[src.rank][dst.file] = nullptr;
    }
  }
  
  // Check if the king is in check after the move
  
  // Find the king's position
  Pos kingPos{-1, -1};
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto p = tempGrid[rank][file];
      if (p && p->colour() == playerColour && 
          (p->symbol() == 'K' || p->symbol() == 'k')) {
        kingPos = {file, rank};
        break;
      }
    }
    if (kingPos.file != -1) break;
  }
  
  // Check if any opponent piece can capture the king
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto p = tempGrid[rank][file];
      if (p && p->colour() != playerColour) {
        // We can't use legalMoves here because it would cause infinite recursion
        // Instead, check if the piece could directly attack the king
        bool canAttack = false;
        
        // Check based on piece type
        if (p->symbol() == 'P' || p->symbol() == 'p') {
          // Pawn attacks diagonally
          int direction = (p->colour() == Colour::White) ? 1 : -1;
          if ((rank + direction == kingPos.rank) && 
              (abs(file - kingPos.file) == 1)) {
            canAttack = true;
          }
        } else if (p->symbol() == 'N' || p->symbol() == 'n') {
          // Knight moves in L-shape
          int dx = abs(file - kingPos.file);
          int dy = abs(rank - kingPos.rank);
          if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
            canAttack = true;
          }
        } else if (p->symbol() == 'B' || p->symbol() == 'b' || 
                   p->symbol() == 'Q' || p->symbol() == 'q') {
          // Bishop or Queen can move diagonally
          if (abs(file - kingPos.file) == abs(rank - kingPos.rank)) {
            // Check if path is clear
            int dx = (kingPos.file > file) ? 1 : -1;
            int dy = (kingPos.rank > rank) ? 1 : -1;
            bool pathClear = true;
            for (int x = file + dx, y = rank + dy; 
                 x != kingPos.file && y != kingPos.rank; 
                 x += dx, y += dy) {
              if (tempGrid[y][x]) {
                pathClear = false;
                break;
              }
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (p->symbol() == 'R' || p->symbol() == 'r' || 
            p->symbol() == 'Q' || p->symbol() == 'q') {
          // Rook or Queen can move horizontally or vertically
          if (file == kingPos.file || rank == kingPos.rank) {
            // Check if path is clear
            bool pathClear = true;
            if (file == kingPos.file) {
              // Vertical movement
              int step = (kingPos.rank > rank) ? 1 : -1;
              for (int y = rank + step; y != kingPos.rank; y += step) {
                if (tempGrid[y][file]) {
                  pathClear = false;
                  break;
                }
              }
            } else {
              // Horizontal movement
              int step = (kingPos.file > file) ? 1 : -1;
              for (int x = file + step; x != kingPos.file; x += step) {
                if (tempGrid[rank][x]) {
                  pathClear = false;
                  break;
                }
              }
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (p->symbol() == 'K' || p->symbol() == 'k') {
          // King attacks one square in any direction
          if (abs(file - kingPos.file) <= 1 && abs(rank - kingPos.rank) <= 1) {
            canAttack = true;
          }
        }
        
        if (canAttack) {
          return false;  // King can be captured, move is not valid
        }
      }
    }
  }
  
  return true;  // King cannot be captured, move is valid
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

std::shared_ptr<Piece> Board::createPromotedPiece(char pieceType, Colour c) {
  switch (toupper(pieceType)) {
    case 'Q': return std::make_shared<Queen>(c);
    case 'R': return std::make_shared<Rook>(c);
    case 'B': return std::make_shared<Bishop>(c);
    case 'N': return std::make_shared<Knight>(c);
    default: return std::make_shared<Queen>(c); // Default to Queen if invalid
  }
}

bool Board::move(Pos src, Pos dst, char promotionPiece) {
  // Check if source position is valid and has a piece
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if it's the correct player's turn
  if (piece->colour() != currentTurn) return false;
  
  // Check for special moves
  bool isCastling = canCastle(src, dst);
  bool isEnPassant = isEnPassantCapture(src, dst);
  
  // For regular moves, check if the destination is a legal move
  if (!isCastling && !isEnPassant) {
    // Get legal moves for the piece
    auto legalMoves = piece->legalMoves(*this, src);
    
    // Check if the destination is a legal move
    if (std::find_if(legalMoves.begin(), legalMoves.end(),
                    [dst](const Pos& p) { return p.file == dst.file && p.rank == dst.rank; }) 
        == legalMoves.end()) {
      return false;
    }
  }
  
  // Check if the move would leave the player in check
  if (!simulateMove(src, dst, currentTurn)) {
    return false;
  }
  
  // Perform the move based on its type
  if (isCastling) {
    performCastling(src, dst);
  } else if (isEnPassant) {
    performEnPassant(src, dst);
  } else {
    // Check if this is a valid pawn promotion
    bool isPawnPromotion = false;
    if (auto pawn = dynamic_cast<Pawn*>(piece.get())) {
      if ((piece->colour() == Colour::White && dst.rank == 7) ||
          (piece->colour() == Colour::Black && dst.rank == 0)) {
        isPawnPromotion = true;
      }
    }
    
    // Make the move
    if (isPawnPromotion) {
      grid[dst.rank][dst.file] = createPromotedPiece(promotionPiece, piece->colour());
    } else {
      grid[dst.rank][dst.file] = piece;
    }
    grid[src.rank][src.file] = nullptr;
  }
  
  // Update tracking for special moves
  updateSpecialMoveTracking(src, dst, piece);
  
  // Switch turns
  currentTurn = (currentTurn == Colour::White) ? Colour::Black : Colour::White;
  
  return true;
}

bool Board::move(Pos src, Pos dst) {
  // For backward compatibility, default to Queen promotion
  return move(src, dst, 'Q');
}

bool Board::isInCheck(Colour c) const {
  // Find the king's position
  Pos kingPos{-1, -1};
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece && piece->colour() == c && 
          (piece->symbol() == 'K' || piece->symbol() == 'k')) {
        kingPos = {file, rank};
        break;
      }
    }
    if (kingPos.file != -1) break;
  }
  
  // Check if the king's position is under attack
  return isSquareAttacked(kingPos, c);
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

bool Board::isCastlingMove(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if this is a king
  bool isKing = (piece->symbol() == 'K' || piece->symbol() == 'k');
  if (!isKing) return false;
  
  // Check if the king is moving two squares horizontally
  if (src.rank != dst.rank) return false;
  if (abs(dst.file - src.file) != 2) return false;
  
  // Check if the king is in its initial position
  bool isWhiteKing = (piece->colour() == Colour::White);
  if (isWhiteKing) {
    if (src.file != 4 || src.rank != 0 || whiteKingMoved) return false;
  } else {
    if (src.file != 4 || src.rank != 7 || blackKingMoved) return false;
  }
  
  // Check if the rook is in its initial position
  bool isKingSideCastling = (dst.file > src.file);
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = isWhiteKing ? 0 : 7;
  auto rook = pieceAt({rookFile, rookRank});
  
  if (!rook || (rook->symbol() != 'R' && rook->symbol() != 'r')) return false;
  if (rook->colour() != piece->colour()) return false;
  
  // Check if the rook has moved
  if (isWhiteKing) {
    if (isKingSideCastling && whiteRookHMoved) return false;
    if (!isKingSideCastling && whiteRookAMoved) return false;
  } else {
    if (isKingSideCastling && blackRookHMoved) return false;
    if (!isKingSideCastling && blackRookAMoved) return false;
  }
  
  // Check if the path between king and rook is clear
  int step = isKingSideCastling ? 1 : -1;
  for (int f = src.file + step; f != rookFile; f += step) {
    if (pieceAt({f, src.rank})) return false;
  }
  
  // Check if the king is in check
  if (isInCheck(piece->colour())) return false;
  
  // Check if the king passes through or ends up in check
  for (int f = src.file + step; f != src.file + 3*step; f += step) {
    if (f < 0 || f > 7) break;
    
    // Simulate the king at this position and check if it would be in check
    Board tempBoard = *this;
    tempBoard.grid[src.rank][src.file] = nullptr;
    tempBoard.grid[src.rank][f] = piece;
    if (tempBoard.isInCheck(piece->colour())) return false;
  }
  
  return true;
}

bool Board::isEnPassantCapture(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if this is a pawn
  bool isPawn = (piece->symbol() == 'P' || piece->symbol() == 'p');
  if (!isPawn) return false;
  
  // Check if this is a diagonal move (capture)
  if (abs(dst.file - src.file) != 1) return false;
  
  int direction = (piece->colour() == Colour::White) ? 1 : -1;
  if (dst.rank - src.rank != direction) return false;
  
  // Check if the destination square is empty
  if (pieceAt(dst)) return false;
  
  // Check if there's a pawn that just made a double move
  if (lastPawnDoubleMove.file == dst.file && lastPawnDoubleMove.rank == src.rank) {
    return true;
  }
  
  return false;
}

bool Board::canEnPassantCapture(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if this is a pawn
  bool isPawn = (piece->symbol() == 'P' || piece->symbol() == 'p');
  if (!isPawn) return false;
  
  // Check if this is a diagonal move (capture)
  if (abs(dst.file - src.file) != 1) return false;
  
  int direction = (piece->colour() == Colour::White) ? 1 : -1;
  if (dst.rank - src.rank != direction) return false;
  
  // Check if the destination square is empty
  if (pieceAt(dst)) return false;
  
  // Check if there's a pawn that just made a double move
  if (lastPawnDoubleMove.file == dst.file && lastPawnDoubleMove.rank == src.rank) {
    return true;
  }
  
  return false;
}

void Board::performCastling(Pos src, Pos dst) {
  auto king = pieceAt(src);
  bool isKingSideCastling = (dst.file > src.file);
  
  // Determine rook position
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = src.rank;
  auto rook = pieceAt({rookFile, rookRank});
  
  // Determine new rook position
  int newRookFile = isKingSideCastling ? 5 : 3;
  
  // Move the king
  grid[dst.rank][dst.file] = king;
  grid[src.rank][src.file] = nullptr;
  
  // Move the rook
  grid[rookRank][newRookFile] = rook;
  grid[rookRank][rookFile] = nullptr;
  
  // Update tracking for castling
  if (king->colour() == Colour::White) {
    whiteKingMoved = true;
    if (isKingSideCastling) {
      whiteRookHMoved = true;
    } else {
      whiteRookAMoved = true;
    }
  } else {
    blackKingMoved = true;
    if (isKingSideCastling) {
      blackRookHMoved = true;
    } else {
      blackRookAMoved = true;
    }
  }
}

void Board::performEnPassant(Pos src, Pos dst) {
  auto pawn = pieceAt(src);
  
  // Move the pawn
  grid[dst.rank][dst.file] = pawn;
  grid[src.rank][src.file] = nullptr;
  
  // Remove the captured pawn (which is on the same file as dst but on the same rank as src)
  grid[src.rank][dst.file] = nullptr;
}

void Board::updateSpecialMoveTracking(Pos src, Pos dst, std::shared_ptr<Piece> piece) {
  // Update castling tracking
  if (piece->symbol() == 'K') {
    whiteKingMoved = true;
  } else if (piece->symbol() == 'k') {
    blackKingMoved = true;
  } else if (piece->symbol() == 'R') {
    if (src.rank == 0) {
      if (src.file == 0) whiteRookAMoved = true;
      if (src.file == 7) whiteRookHMoved = true;
    }
  } else if (piece->symbol() == 'r') {
    if (src.rank == 7) {
      if (src.file == 0) blackRookAMoved = true;
      if (src.file == 7) blackRookHMoved = true;
    }
  }
  
  // Update en passant tracking
  if (piece->symbol() == 'P' || piece->symbol() == 'p') {
    // Check if this is a two-square pawn move
    if (abs(dst.rank - src.rank) == 2) {
      lastPawnDoubleMove = dst;
    } else {
      lastPawnDoubleMove = {-1, -1};  // Reset
    }
  } else {
    lastPawnDoubleMove = {-1, -1};  // Reset for non-pawn moves
  }
} 

bool Board::isSquareAttacked(Pos square, Colour defendingColour) const {
  // Check if any opponent piece can attack this square
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece && piece->colour() != defendingColour) {
        // Check if this piece can attack the square
        bool canAttack = false;
        Pos piecePos{file, rank};
        
        // Check based on piece type
        if (piece->symbol() == 'P' || piece->symbol() == 'p') {
          // Pawn attacks diagonally
          int direction = (piece->colour() == Colour::White) ? 1 : -1;
          if ((rank + direction == square.rank) && 
              (abs(file - square.file) == 1)) {
            canAttack = true;
          }
        } else if (piece->symbol() == 'N' || piece->symbol() == 'n') {
          // Knight moves in L-shape
          int dx = abs(file - square.file);
          int dy = abs(rank - square.rank);
          if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
            canAttack = true;
          }
        } else if (piece->symbol() == 'B' || piece->symbol() == 'b' || 
                   piece->symbol() == 'Q' || piece->symbol() == 'q') {
          // Bishop or Queen can move diagonally
          if (abs(file - square.file) == abs(rank - square.rank)) {
            // Check if path is clear
            int dx = (square.file > file) ? 1 : -1;
            int dy = (square.rank > rank) ? 1 : -1;
            bool pathClear = true;
            for (int x = file + dx, y = rank + dy; 
                 x != square.file && y != square.rank; 
                 x += dx, y += dy) {
              if (pieceAt({x, y})) {
                pathClear = false;
                break;
              }
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (piece->symbol() == 'R' || piece->symbol() == 'r' || 
            piece->symbol() == 'Q' || piece->symbol() == 'q') {
          // Rook or Queen can move horizontally or vertically
          if (file == square.file || rank == square.rank) {
            // Check if path is clear
            bool pathClear = true;
            if (file == square.file) {
              // Vertical movement
              int step = (square.rank > rank) ? 1 : -1;
              for (int y = rank + step; y != square.rank; y += step) {
                if (pieceAt({file, y})) {
                  pathClear = false;
                  break;
                }
              }
            } else {
              // Horizontal movement
              int step = (square.file > file) ? 1 : -1;
              for (int x = file + step; x != square.file; x += step) {
                if (pieceAt({x, rank})) {
                  pathClear = false;
                  break;
                }
              }
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (piece->symbol() == 'K' || piece->symbol() == 'k') {
          // King attacks one square in any direction
          if (abs(file - square.file) <= 1 && abs(rank - square.rank) <= 1) {
            canAttack = true;
          }
        }
        
        if (canAttack) {
          return true;
        }
      }
    }
  }
  
  return false;
}

bool Board::hasKingMoved(Colour c) const {
  return (c == Colour::White) ? whiteKingMoved : blackKingMoved;
}

bool Board::hasRookMoved(Colour c, bool kingSide) const {
  if (c == Colour::White) {
    return kingSide ? whiteRookHMoved : whiteRookAMoved;
  } else {
    return kingSide ? blackRookHMoved : blackRookAMoved;
  }
}

bool Board::isPathClear(Pos from, Pos to) const {
  // Determine the direction of movement
  int fileDir = (to.file > from.file) ? 1 : (to.file < from.file) ? -1 : 0;
  int rankDir = (to.rank > from.rank) ? 1 : (to.rank < from.rank) ? -1 : 0;
  
  // Start from the square after 'from' and check each square until 'to' (exclusive)
  Pos current = from;
  while (true) {
    current.file += fileDir;
    current.rank += rankDir;
    
    // Stop when we reach the destination
    if (current.file == to.file && current.rank == to.rank) {
      break;
    }
    
    // If there's a piece in the path, the path is not clear
    if (isValidPos(current) && pieceAt(current)) {
      return false;
    }
  }
  
  return true;
}

bool Board::canCastle(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  // Check if this is a king
  bool isKing = (piece->symbol() == 'K' || piece->symbol() == 'k');
  if (!isKing) return false;
  
  // Check if the king is moving two squares horizontally
  if (src.rank != dst.rank) return false;
  if (abs(dst.file - src.file) != 2) return false;
  
  // Check if the king is in its initial position
  bool isWhiteKing = (piece->colour() == Colour::White);
  if (isWhiteKing) {
    if (src.file != 4 || src.rank != 0 || whiteKingMoved) return false;
  } else {
    if (src.file != 4 || src.rank != 7 || blackKingMoved) return false;
  }
  
  // Check if the rook is in its initial position
  bool isKingSideCastling = (dst.file > src.file);
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = isWhiteKing ? 0 : 7;
  auto rook = pieceAt({rookFile, rookRank});
  
  if (!rook || (rook->symbol() != 'R' && rook->symbol() != 'r')) return false;
  if (rook->colour() != piece->colour()) return false;
  
  // Check if the rook has moved
  if (isWhiteKing) {
    if (isKingSideCastling && whiteRookHMoved) return false;
    if (!isKingSideCastling && whiteRookAMoved) return false;
  } else {
    if (isKingSideCastling && blackRookHMoved) return false;
    if (!isKingSideCastling && blackRookAMoved) return false;
  }
  
  // Check if the path between king and rook is clear
  if (!isPathClear(src, {rookFile, rookRank})) return false;
  
  // Check if the king is in check
  if (isInCheck(piece->colour())) return false;
  
  // Check if the king passes through or ends up in check
  int step = isKingSideCastling ? 1 : -1;
  for (int f = src.file + step; f != src.file + 3*step; f += step) {
    if (f < 0 || f > 7) break;
    
    if (isSquareAttacked({f, src.rank}, piece->colour())) return false;
  }
  
  return true;
} 

Colour Board::getCurrentTurn() const {
  return currentTurn;
} 