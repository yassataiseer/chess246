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
  ~Board();
  bool move(Pos src, Pos dst);
  bool move(Pos src, Pos dst, char promotionPiece);
  void draw(std::ostream& os) const;
  std::shared_ptr<Piece> pieceAt(Pos p) const;
  bool isInCheck(Colour c) const;
  bool isCheckmate(Colour c) const;
  bool isStalemate(Colour c) const;
  bool isValidPos(Pos p) const;
  bool canEnPassantCapture(Pos src, Pos dst) const;
  bool canCastle(Pos src, Pos dst) const;
  Colour getCurrentTurn() const;
  
  void clearBoard();
  void placePiece(Pos pos, char pieceType, Colour colour);
  void removePiece(Pos pos);
  void setCurrentTurn(Colour c);
  
  bool hasKingMoved(Colour c) const;
  bool hasRookMoved(Colour c, bool kingSide) const;
  bool isPathClear(Pos from, Pos to) const;
  bool isSquareAttacked(Pos square, Colour defendingColour) const;
  
  bool simulateMove(Pos src, Pos dst, Colour playerColour) const;

private:
  std::vector<std::vector<std::shared_ptr<Piece>>> grid;
  Colour currentTurn;
  
  bool whiteKingMoved = false;
  bool blackKingMoved = false;
  bool whiteRookAMoved = false;
  bool whiteRookHMoved = false;
  bool blackRookAMoved = false;
  bool blackRookHMoved = false;
  
  Pos lastPawnDoubleMove = {-1, -1};

  std::shared_ptr<Piece> createPromotedPiece(char pieceType, Colour c);
  
  bool isCastlingMove(Pos src, Pos dst) const;
  bool isEnPassantCapture(Pos src, Pos dst) const;
  void performCastling(Pos src, Pos dst);
  void performEnPassant(Pos src, Pos dst);
  void updateSpecialMoveTracking(Pos src, Pos dst, std::shared_ptr<Piece> piece);
};

#endif 
