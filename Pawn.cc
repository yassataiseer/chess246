#include "Pawn.h"
#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>
#include <memory>

Pawn::Pawn(Colour c): Piece{c} {}

char Pawn::symbol() const {
  return colour() == Colour::White ? 'P' : 'p';
}

std::vector<Pos> Pawn::legalMoves(Board const& b, Pos from) const {
  std::vector<Pos> moves;
  int direction = (colour() == Colour::White) ? 1 : -1;
  
  Pos oneStep{from.file, from.rank + direction};
  if (b.isValidPos(oneStep) && !b.pieceAt(oneStep)) {
    moves.push_back(oneStep);
    
    if ((colour() == Colour::White && from.rank == 1) || 
        (colour() == Colour::Black && from.rank == 6)) {
      Pos twoStep{from.file, from.rank + 2 * direction};
      if (b.isValidPos(twoStep) && !b.pieceAt(twoStep)) {
        moves.push_back(twoStep);
      }
    }
  }
  
  for (int df : {-1, 1}) {
    Pos capture{from.file + df, from.rank + direction};
    if (b.isValidPos(capture)) {
      auto piece = b.pieceAt(capture);
      if (piece && piece->colour() != colour()) {
        moves.push_back(capture);
      }
      
      if (!piece && b.canEnPassantCapture(from, capture)) {
        moves.push_back(capture);
      }
    }
  }
  
  return moves;
} 
