#ifndef PIECE_H
#define PIECE_H

#include "Colour.h"
#include "Pos.h"
#include <vector>
#include <memory>

// Forward declaration
class Board;

class Piece {
public:
  Piece(Colour c);
  virtual ~Piece();
  Colour colour() const;
  virtual char symbol() const = 0;
  virtual std::vector<Pos> legalMoves(Board const& b, Pos from) const = 0;
private:
  Colour c;
};

#endif // PIECE_H 