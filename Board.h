#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include "Pos.h"
#include "Colour.h"
#include <vector>
#include <memory>
#include <ostream>

class Board {
public:
  Board();
  bool move(Pos src, Pos dst);             // applies move if legal
  void draw(std::ostream& os) const;       // ASCII draw
  std::shared_ptr<Piece> pieceAt(Pos p) const;
  bool isInCheck(Colour c) const;
  bool isValidPos(Pos p) const;
private:
  std::vector<std::vector<std::shared_ptr<Piece>>> grid;
  Colour currentTurn;
};

#endif // BOARD_H 