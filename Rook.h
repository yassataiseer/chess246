#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

class Rook : public Piece {
public:
  Rook(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
};

#endif 
