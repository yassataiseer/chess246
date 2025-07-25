#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include "Board.h"
#include <vector>
#include <memory>

class Pawn : public Piece {
public:
  Pawn(Colour c);
  char symbol() const override;
  std::vector<Pos> legalMoves(Board const& b, Pos from) const override;
private:
  bool hasMoved = false;
};

#endif 
