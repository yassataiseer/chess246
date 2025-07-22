#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

class Bishop : public Piece {
public:
  Bishop(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
};

#endif // BISHOP_H 