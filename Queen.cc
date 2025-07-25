#include "Queen.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

Queen::Queen(Colour c): Piece{c} {}

char Queen::symbol() const {
  return colour() == Colour::White ? 'Q' : 'q';
}

std::vector<Pos> Queen::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  
  const std::vector<std::pair<int, int>> directions = {
    {1, 0}, {1, -1}, {0, -1}, {-1, -1},
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1}
  };
  
  for (const auto& [df, dr] : directions) {
    for (int i = 1; i < 8; ++i) {
      Pos dest{from.file + i * df, from.rank + i * dr};
      if (!b.isValidPos(dest)) break;
      
      auto piece = b.pieceAt(dest);
      if (!piece) {
        moves.push_back(dest);
      } else {
        if (piece->colour() != colour()) {
          moves.push_back(dest);
        }
        break;
      }
    }
  }
  
  return moves;
} 