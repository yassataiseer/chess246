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
  
  return moves;
} 