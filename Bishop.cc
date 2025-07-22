#include "Bishop.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

Bishop::Bishop(Colour c): Piece{c} {}

char Bishop::symbol() const {
  return colour() == Colour::White ? 'B' : 'b';
}

std::vector<Pos> Bishop::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  
  // Diagonal directions: top-right, bottom-right, bottom-left, top-left
  const std::vector<std::pair<int, int>> directions = {
    {1, 1}, {1, -1}, {-1, -1}, {-1, 1}
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
        break; // Can't move past a piece
      }
    }
  }
  
  return moves;
} 