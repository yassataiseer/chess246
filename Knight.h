#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>

class Knight : public Piece {
public:
  Knight(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
};

#endif // KNIGHT_H 