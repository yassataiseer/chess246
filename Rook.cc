#include "Rook.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

Rook::Rook(Colour c): Piece{c} {}

char Rook::symbol() const {
  return colour() == Colour::White ? 'R' : 'r';
}

std::vector<Pos> Rook::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  
  // Horizontal and vertical directions: right, down, left, up
  const std::vector<std::pair<int, int>> directions = {
    {1, 0}, {0, -1}, {-1, 0}, {0, 1}
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