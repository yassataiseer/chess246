#include "Piece.h"
#include "Colour.h"

Piece::Piece(Colour c): c{c} {}

Piece::~Piece() {}

Colour Piece::colour() const {
  return c;
} 