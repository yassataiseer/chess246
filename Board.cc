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
#include <iostream>

Board::Board() : grid(8, std::vector<std::shared_ptr<Piece>>(8, nullptr)), currentTurn{Colour::White} {
  for (int file = 0; file < 8; ++file) {
    grid[1][file] = std::make_shared<Pawn>(Colour::White);
    grid[6][file] = std::make_shared<Pawn>(Colour::Black);
  }
  
  grid[0][0] = std::make_shared<Rook>(Colour::White);
  grid[0][1] = std::make_shared<Knight>(Colour::White);
  grid[0][2] = std::make_shared<Bishop>(Colour::White);
  grid[0][3] = std::make_shared<Queen>(Colour::White);
  grid[0][4] = std::make_shared<King>(Colour::White);
  grid[0][5] = std::make_shared<Bishop>(Colour::White);
  grid[0][6] = std::make_shared<Knight>(Colour::White);
  grid[0][7] = std::make_shared<Rook>(Colour::White);
  
  grid[7][0] = std::make_shared<Rook>(Colour::Black);
  grid[7][1] = std::make_shared<Knight>(Colour::Black);
  grid[7][2] = std::make_shared<Bishop>(Colour::Black);
  grid[7][3] = std::make_shared<Queen>(Colour::Black);
  grid[7][4] = std::make_shared<King>(Colour::Black);
  grid[7][5] = std::make_shared<Bishop>(Colour::Black);
  grid[7][6] = std::make_shared<Knight>(Colour::Black);
  grid[7][7] = std::make_shared<Rook>(Colour::Black);
}

Board::~Board() {
}

bool Board::isValidPos(Pos p) const {
  return p.file >= 0 && p.file < 8 && p.rank >= 0 && p.rank < 8;
}

std::shared_ptr<Piece> Board::pieceAt(Pos p) const {
  if (!isValidPos(p)) return nullptr;
  return grid[p.rank][p.file];
}

bool Board::simulateMove(Pos src, Pos dst, Colour playerColour) const {
  auto tempGrid = grid;
  
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  tempGrid[dst.rank][dst.file] = piece;
  tempGrid[src.rank][src.file] = nullptr;

  if (piece->symbol() == 'P' || piece->symbol() == 'p') {
    if (abs(dst.file - src.file) == 1 && abs(dst.rank - src.rank) == 1) {
      if (!pieceAt(dst)) {
        tempGrid[src.rank][dst.file] = nullptr;
      }
    }
  }
  
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
  
  if (kingPos.file == -1) {
    return false;
  }
  
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto p = tempGrid[rank][file];
      if (p && p->colour() != playerColour) {
        bool canAttack = false;
        
        if (p->symbol() == 'P' || p->symbol() == 'p') {
          int direction = (p->colour() == Colour::White) ? 1 : -1;
          if ((rank + direction == kingPos.rank) && 
              (abs(file - kingPos.file) == 1)) {
            canAttack = true;
          }
        } else if (p->symbol() == 'N' || p->symbol() == 'n') {
          int dx = abs(file - kingPos.file);
          int dy = abs(rank - kingPos.rank);
          if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
            canAttack = true;
          }
        } else if (p->symbol() == 'B' || p->symbol() == 'b' || 
                   p->symbol() == 'Q' || p->symbol() == 'q') {
          if (abs(file - kingPos.file) == abs(rank - kingPos.rank)) {
            int dx = (kingPos.file > file) ? 1 : -1;
            int dy = (kingPos.rank > rank) ? 1 : -1;
            bool pathClear = true;
            
            int x = file + dx;
            int y = rank + dy;
            while (x != kingPos.file && y != kingPos.rank) {
              if (tempGrid[y][x]) {
                pathClear = false;
                break;
              }
              x += dx;
              y += dy;
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (p->symbol() == 'R' || p->symbol() == 'r' || 
            p->symbol() == 'Q' || p->symbol() == 'q') {
          if (file == kingPos.file || rank == kingPos.rank) {
            bool pathClear = true;
            if (file == kingPos.file) {
              int step = (kingPos.rank > rank) ? 1 : -1;
              for (int y = rank + step; y != kingPos.rank; y += step) {
                if (tempGrid[y][file]) {
                  pathClear = false;
                  break;
                }
              }
            } else {
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
          if (abs(file - kingPos.file) <= 1 && abs(rank - kingPos.rank) <= 1) {
            canAttack = true;
          }
        }
        
        if (canAttack) {
          return false;
        }
      }
    }
  }
  
  return true;
}

bool Board::isCheckmate(Colour c) const {
  if (!isInCheck(c)) {
    return false;
  }
  
  for (int srcRank = 0; srcRank < 8; ++srcRank) {
    for (int srcFile = 0; srcFile < 8; ++srcFile) {
      Pos src{srcFile, srcRank};
      auto piece = pieceAt(src);
      
      if (!piece || piece->colour() != c) {
        continue;
      }
      
      auto legalMoves = piece->legalMoves(*this, src);
      
      for (const auto& dst : legalMoves) {
        if (simulateMove(src, dst, c)) {
          return false;
        }
      }
    }
  }
  
  return true;
}

bool Board::isStalemate(Colour c) const {
  if (isInCheck(c)) {
    return false;
  }
  
  for (int srcRank = 0; srcRank < 8; ++srcRank) {
    for (int srcFile = 0; srcFile < 8; ++srcFile) {
      Pos src{srcFile, srcRank};
      auto piece = pieceAt(src);
      
      if (!piece || piece->colour() != c) {
        continue;
      }
      
      auto legalMoves = piece->legalMoves(*this, src);
      
      for (const auto& dst : legalMoves) {
        if (simulateMove(src, dst, c)) {
          return false;
        }
      }
    }
  }
  
  return true;
}

std::shared_ptr<Piece> Board::createPromotedPiece(char pieceType, Colour c) {
  switch (toupper(pieceType)) {
    case 'Q': return std::make_shared<Queen>(c);
    case 'R': return std::make_shared<Rook>(c);
    case 'B': return std::make_shared<Bishop>(c);
    case 'N': return std::make_shared<Knight>(c);
    default: return std::make_shared<Queen>(c);
  }
}

bool Board::move(Pos src, Pos dst, char promotionPiece) {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  if (piece->colour() != currentTurn) return false;
  
  bool isCastling = canCastle(src, dst);
  bool isEnPassant = isEnPassantCapture(src, dst);
  
  if (!isCastling && !isEnPassant) {
    auto legalMoves = piece->legalMoves(*this, src);
    
    if (std::find_if(legalMoves.begin(), legalMoves.end(),
                    [dst](const Pos& p) { return p.file == dst.file && p.rank == dst.rank; }) 
        == legalMoves.end()) {
      return false;
    }
  }
  
  if (!simulateMove(src, dst, currentTurn)) {
    return false;
  }
  
  if (isCastling) {
    performCastling(src, dst);
  } else if (isEnPassant) {
    performEnPassant(src, dst);
  } else {
    bool isPawnPromotion = false;
    if (dynamic_cast<Pawn*>(piece.get())) {
      if ((piece->colour() == Colour::White && dst.rank == 7) ||
          (piece->colour() == Colour::Black && dst.rank == 0)) {
        isPawnPromotion = true;
      }
    }
    
    if (isPawnPromotion) {
      grid[dst.rank][dst.file] = createPromotedPiece(promotionPiece, piece->colour());
    } else {
      grid[dst.rank][dst.file] = piece;
    }
    grid[src.rank][src.file] = nullptr;
  }
  
  updateSpecialMoveTracking(src, dst, piece);
  
  currentTurn = (currentTurn == Colour::White) ? Colour::Black : Colour::White;
  
  return true;
}

bool Board::move(Pos src, Pos dst) {
  return move(src, dst, 'Q');
}

bool Board::isInCheck(Colour c) const {
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
  
  bool isKing = (piece->symbol() == 'K' || piece->symbol() == 'k');
  if (!isKing) return false;
  
  if (src.rank != dst.rank) return false;
  if (abs(dst.file - src.file) != 2) return false;
  
  bool isWhiteKing = (piece->colour() == Colour::White);
  if (isWhiteKing) {
    if (src.file != 4 || src.rank != 0 || whiteKingMoved) return false;
  } else {
    if (src.file != 4 || src.rank != 7 || blackKingMoved) return false;
  }
  
  bool isKingSideCastling = (dst.file > src.file);
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = isWhiteKing ? 0 : 7;
  auto rook = pieceAt({rookFile, rookRank});
  
  if (!rook || (rook->symbol() != 'R' && rook->symbol() != 'r')) return false;
  if (rook->colour() != piece->colour()) return false;
  
  if (isWhiteKing) {
    if (isKingSideCastling && whiteRookHMoved) return false;
    if (!isKingSideCastling && whiteRookAMoved) return false;
  } else {
    if (isKingSideCastling && blackRookHMoved) return false;
    if (!isKingSideCastling && blackRookAMoved) return false;
  }
  
  int step = isKingSideCastling ? 1 : -1;
  for (int f = src.file + step; f != rookFile; f += step) {
    if (pieceAt({f, src.rank})) return false;
  }
  
  if (isInCheck(piece->colour())) return false;
  
  for (int f = src.file + step; f != src.file + 3*step; f += step) {
    if (f < 0 || f > 7) break;
    
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
  
  bool isPawn = (piece->symbol() == 'P' || piece->symbol() == 'p');
  if (!isPawn) return false;
  
  if (abs(dst.file - src.file) != 1) return false;
  
  int direction = (piece->colour() == Colour::White) ? 1 : -1;
  if (dst.rank - src.rank != direction) return false;
  
  if (pieceAt(dst)) return false;
  
  if (lastPawnDoubleMove.file == dst.file && lastPawnDoubleMove.rank == src.rank) {
    return true;
  }
  
  return false;
}

bool Board::canEnPassantCapture(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  bool isPawn = (piece->symbol() == 'P' || piece->symbol() == 'p');
  if (!isPawn) return false;
  
  if (abs(dst.file - src.file) != 1) return false;
  
  int direction = (piece->colour() == Colour::White) ? 1 : -1;
  if (dst.rank - src.rank != direction) return false;
  
  if (pieceAt(dst)) return false;
  
  if (lastPawnDoubleMove.file == dst.file && lastPawnDoubleMove.rank == src.rank) {
    return true;
  }
  
  return false;
}

void Board::performCastling(Pos src, Pos dst) {
  auto king = pieceAt(src);
  bool isKingSideCastling = (dst.file > src.file);
  
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = src.rank;
  auto rook = pieceAt({rookFile, rookRank});
  
  int newRookFile = isKingSideCastling ? 5 : 3;
  
  grid[dst.rank][dst.file] = king;
  grid[src.rank][src.file] = nullptr;
  
  grid[rookRank][newRookFile] = rook;
  grid[rookRank][rookFile] = nullptr;
  
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
  grid[dst.rank][dst.file] = pawn;
  grid[src.rank][src.file] = nullptr;
  grid[src.rank][dst.file] = nullptr;
}

void Board::updateSpecialMoveTracking(Pos src, Pos dst, std::shared_ptr<Piece> piece) {
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
  
  if (piece->symbol() == 'P' || piece->symbol() == 'p') {
    if (abs(dst.rank - src.rank) == 2) {
      lastPawnDoubleMove = dst;
    } else {
      lastPawnDoubleMove = {-1, -1};
    }
  } else {
    lastPawnDoubleMove = {-1, -1};
  }
} 

bool Board::isSquareAttacked(Pos square, Colour defendingColour) const {
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      auto piece = grid[rank][file];
      if (piece && piece->colour() != defendingColour) {
        bool canAttack = false;
        
        if (piece->symbol() == 'P' || piece->symbol() == 'p') {
          int direction = (piece->colour() == Colour::White) ? 1 : -1;
          if ((rank + direction == square.rank) && 
              (abs(file - square.file) == 1)) {
            canAttack = true;
          }
        } else if (piece->symbol() == 'N' || piece->symbol() == 'n') {
          int dx = abs(file - square.file);
          int dy = abs(rank - square.rank);
          if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) {
            canAttack = true;
          }
        } else if (piece->symbol() == 'B' || piece->symbol() == 'b' || 
                   piece->symbol() == 'Q' || piece->symbol() == 'q') {
          if (abs(file - square.file) == abs(rank - square.rank)) {
            int dx = (square.file > file) ? 1 : -1;
            int dy = (square.rank > rank) ? 1 : -1;
            bool pathClear = true;
            
            int x = file + dx;
            int y = rank + dy;
            while (x != square.file && y != square.rank) {
              if (pieceAt({x, y})) {
                pathClear = false;
                break;
              }
              x += dx;
              y += dy;
            }
            if (pathClear) canAttack = true;
          }
        }
        
        if (piece->symbol() == 'R' || piece->symbol() == 'r' || 
            piece->symbol() == 'Q' || piece->symbol() == 'q') {
          if (file == square.file || rank == square.rank) {
            bool pathClear = true;
            if (file == square.file) {
              int step = (square.rank > rank) ? 1 : -1;
              for (int y = rank + step; y != square.rank; y += step) {
                if (pieceAt({file, y})) {
                  pathClear = false;
                  break;
                }
              }
            } else {
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
  int fileDir = (to.file > from.file) ? 1 : (to.file < from.file) ? -1 : 0;
  int rankDir = (to.rank > from.rank) ? 1 : (to.rank < from.rank) ? -1 : 0;
  
  Pos current = from;
  while (true) {
    current.file += fileDir;
    current.rank += rankDir;
    
    if (current.file == to.file && current.rank == to.rank) {
      break;
    }
    
    if (isValidPos(current) && pieceAt(current)) {
      return false;
    }
  }
  
  return true;
}

bool Board::canCastle(Pos src, Pos dst) const {
  auto piece = pieceAt(src);
  if (!piece) return false;
  
  bool isKing = (piece->symbol() == 'K' || piece->symbol() == 'k');
  if (!isKing) return false;
  
  if (src.rank != dst.rank) return false;
  if (abs(dst.file - src.file) != 2) return false;
  
  bool isWhiteKing = (piece->colour() == Colour::White);
  if (isWhiteKing) {
    if (src.file != 4 || src.rank != 0 || whiteKingMoved) return false;
  } else {
    if (src.file != 4 || src.rank != 7 || blackKingMoved) return false;
  }
  
  bool isKingSideCastling = (dst.file > src.file);
  int rookFile = isKingSideCastling ? 7 : 0;
  int rookRank = isWhiteKing ? 0 : 7;
  auto rook = pieceAt({rookFile, rookRank});
  
  if (!rook || (rook->symbol() != 'R' && rook->symbol() != 'r')) return false;
  if (rook->colour() != piece->colour()) return false;
  
  if (isWhiteKing) {
    if (isKingSideCastling && whiteRookHMoved) return false;
    if (!isKingSideCastling && whiteRookAMoved) return false;
  } else {
    if (isKingSideCastling && blackRookHMoved) return false;
    if (!isKingSideCastling && blackRookAMoved) return false;
  }
  
  int step = isKingSideCastling ? 1 : -1;
  for (int f = src.file + step; f != rookFile; f += step) {
    if (pieceAt({f, src.rank})) return false;
  }
  
  if (isInCheck(piece->colour())) {
    std::cout << "Cannot castle while in check." << std::endl;
    return false;
  }
  
  for (int f = src.file + step; f != src.file + 3*step; f += step) {
    if (f < 0 || f > 7) break;
    
    Board tempBoard = *this;
    tempBoard.grid[src.rank][src.file] = nullptr;
    tempBoard.grid[src.rank][f] = piece;
    if (tempBoard.isInCheck(piece->colour())) return false;
  }
  
  return true;
} 

Colour Board::getCurrentTurn() const {
  return currentTurn;
}

void Board::clearBoard() {
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      grid[rank][file] = nullptr;
    }
  }
  
  currentTurn = Colour::White;
  whiteKingMoved = false;
  blackKingMoved = false;
  whiteRookAMoved = false;
  whiteRookHMoved = false;
  blackRookAMoved = false;
  blackRookHMoved = false;
  lastPawnDoubleMove = {-1, -1};
}

void Board::placePiece(Pos pos, char pieceType, Colour colour) {
  if (!isValidPos(pos)) return;
  
  std::shared_ptr<Piece> piece = nullptr;
  
  switch (toupper(pieceType)) {
    case 'P': piece = std::make_shared<Pawn>(colour); break;
    case 'R': piece = std::make_shared<Rook>(colour); break;
    case 'N': piece = std::make_shared<Knight>(colour); break;
    case 'B': piece = std::make_shared<Bishop>(colour); break;
    case 'Q': piece = std::make_shared<Queen>(colour); break;
    case 'K': piece = std::make_shared<King>(colour); break;
    default: return;
  }
  
  grid[pos.rank][pos.file] = piece;
}

void Board::removePiece(Pos pos) {
  if (!isValidPos(pos)) return;
  
  grid[pos.rank][pos.file] = nullptr;
}

void Board::setCurrentTurn(Colour c) {
  currentTurn = c;
} 
