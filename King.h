#ifndef KING_H
#define KING_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

class King : public Piece {
public:
  King(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
};

#endif 
