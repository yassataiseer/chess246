#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

class Queen : public Piece {
public:
  Queen(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
};

#endif 
