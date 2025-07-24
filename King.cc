#include "King.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

King::King(Colour c): Piece{c} {}

char King::symbol() const {
  return colour() == Colour::White ? 'K' : 'k';
}

std::vector<Pos> King::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  
  // All eight directions: horizontal, vertical, and diagonal (one step only)
  const std::vector<std::pair<int, int>> directions = {
    {1, 0}, {1, -1}, {0, -1}, {-1, -1},
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1}
  };
  
  for (const auto& [df, dr] : directions) {
    Pos dest{from.file + df, from.rank + dr};
    if (b.isValidPos(dest)) {
      auto piece = b.pieceAt(dest);
      if (!piece || piece->colour() != colour()) {
        moves.push_back(dest);
      }
    }
  }
  
  // Check for castling moves
  if (!b.hasKingMoved(colour()) && !b.isInCheck(colour())) {
    // Initial king position
    int rank = (colour() == Colour::White) ? 0 : 7;
    
    // Kingside castling
    if (!b.hasRookMoved(colour(), true)) {
      Pos rookPos{7, rank};
      if (b.isPathClear(from, rookPos) && 
          !b.isSquareAttacked({from.file + 1, rank}, colour()) && 
          !b.isSquareAttacked({from.file + 2, rank}, colour())) {
        moves.push_back({from.file + 2, rank});
      }
    }
    
    // Queenside castling
    if (!b.hasRookMoved(colour(), false)) {
      Pos rookPos{0, rank};
      if (b.isPathClear(from, rookPos) && 
          !b.isSquareAttacked({from.file - 1, rank}, colour()) && 
          !b.isSquareAttacked({from.file - 2, rank}, colour())) {
        moves.push_back({from.file - 2, rank});
      }
    }
  }
  
  return moves;
} 