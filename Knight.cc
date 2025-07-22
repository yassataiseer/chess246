#include "Knight.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

Knight::Knight(Colour c): Piece{c} {}

char Knight::symbol() const {
  return colour() == Colour::White ? 'N' : 'n';
}

std::vector<Pos> Knight::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  
  // All possible knight moves
  const std::vector<std::pair<int, int>> offsets = {
    {1, 2}, {2, 1}, {2, -1}, {1, -2},
    {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
  };
  
  for (const auto& [df, dr] : offsets) {
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