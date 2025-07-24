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
  bool move(Pos src, Pos dst, char promotionPiece); // applies move with promotion if legal
  void draw(std::ostream& os) const;       // ASCII draw
  std::shared_ptr<Piece> pieceAt(Pos p) const;
  bool isInCheck(Colour c) const;
  bool isCheckmate(Colour c) const;        // check if player is in checkmate
  bool isValidPos(Pos p) const;
private:
  std::vector<std::vector<std::shared_ptr<Piece>>> grid;
  Colour currentTurn;
  bool simulateMove(Pos src, Pos dst, Colour playerColour) const; // simulate move to check if it leaves player in check
  std::shared_ptr<Piece> createPromotedPiece(char pieceType, Colour c); // create a piece for promotion
};

#endif // BOARD_H 