#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "Board.h"
#include "Pos.h"
#include "Piece.h"
#include "King.h"
#include "Pawn.h"
#include "Rook.h"
#include "Knight.h"
#include "Bishop.h"
#include "Queen.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <random>

class Move;

enum class PlayerType { Human, Computer };
enum class ComputerLevel { Level1, Level2, Level3, Level4 };

struct Move {
  Pos from;
  Pos to;
  char promotion;
  Move(Pos f, Pos t, char p = '\0') : from(f), to(t), promotion(p) {}
};

class GameController {
public:
  GameController();
  ~GameController();
  void run();

private:
  std::shared_ptr<Board> board;
  bool gameInProgress;
  bool gameOver;
  bool setupMode;
  double whiteScore;
  double blackScore;

  PlayerType whitePlayerType;
  PlayerType blackPlayerType;
  ComputerLevel whiteComputerLevel;
  ComputerLevel blackComputerLevel;
  mutable std::mt19937 rng;

  Display* display;
  Window window;
  GC gc;
  int cellSize;
  bool graphicsActive;

  unsigned long whiteSquareColor;
  unsigned long blackSquareColor;
  unsigned long whitePieceColor;
  unsigned long blackPieceColor;
  unsigned long textColor;

  bool initGraphics();
  void renderGraphics() const;
  bool processGraphicsEvents();
  void closeGraphics();
  void drawPieceSprite(int x, int y, char symbol) const;

  void printFinalScore() const;
  void printScore() const;
  void incrementScore(Colour winner);
  void incrementDrawScore();
  bool processCommand(const std::string& cmd);
  Pos parsePos(const std::string& pos);

  bool processSetupCommand(const std::string& cmd);
  bool validateBoard() const;

  bool isComputerTurn() const;
  void makeComputerMove();
  std::vector<Move> getAllLegalMoves(Colour colour) const;
  Move getRandomMove(const std::vector<Move>& moves) const;
  Move getBestMoveLevel2(const std::vector<Move>& moves) const;
  Move getBestMoveLevel3(const std::vector<Move>& moves) const;
  Move getBestMoveLevel4(const std::vector<Move>& moves) const;
  bool isCapturingMove(const Move& move) const;
  bool isCheckingMove(const Move& move) const;
  bool movePutsInDanger(const Move& move) const;
  int evaluatePosition(const Board& board, Colour perspective) const;
  int getPieceValue(char pieceSymbol) const;
};

#endif 
