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
  bool isStalemate(Colour c) const;        // check if player has no legal moves but is not in check
  bool isValidPos(Pos p) const;
  bool canEnPassantCapture(Pos src, Pos dst) const;  // check if en passant capture is possible
  bool canCastle(Pos src, Pos dst) const;  // check if castling is legal
  Colour getCurrentTurn() const;           // get the current player's color
  
  // Setup mode methods
  void placePiece(Pos pos, char pieceType, Colour colour);  // place a piece during setup
  void removePiece(Pos pos);                               // remove a piece during setup
  void setCurrentTurn(Colour c);                           // set the current player's turn
  void clearBoard();                                       // clear the board for setup mode
  
  // Helper methods for King's legal moves
  bool hasKingMoved(Colour c) const;
  bool hasRookMoved(Colour c, bool kingSide) const;
  bool isPathClear(Pos from, Pos to) const;
  bool isSquareAttacked(Pos square, Colour defendingColour) const;
private:
  std::vector<std::vector<std::shared_ptr<Piece>>> grid;
  Colour currentTurn;
  
  // Tracking for castling
  bool whiteKingMoved = false;
  bool blackKingMoved = false;
  bool whiteRookAMoved = false;  // a-file rook
  bool whiteRookHMoved = false;  // h-file rook
  bool blackRookAMoved = false;  // a-file rook
  bool blackRookHMoved = false;  // h-file rook
  
  // Tracking for en passant
  Pos lastPawnDoubleMove = {-1, -1};  // Invalid position by default
  
  bool simulateMove(Pos src, Pos dst, Colour playerColour) const; // simulate move to check if it leaves player in check
  std::shared_ptr<Piece> createPromotedPiece(char pieceType, Colour c); // create a piece for promotion
  
  // Helper methods for special moves
  bool isCastlingMove(Pos src, Pos dst) const;
  bool isEnPassantCapture(Pos src, Pos dst) const;
  void performCastling(Pos src, Pos dst);
  void performEnPassant(Pos src, Pos dst);
  void updateSpecialMoveTracking(Pos src, Pos dst, std::shared_ptr<Piece> piece);
};

#endif // BOARD_H 