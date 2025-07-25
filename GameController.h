#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "Board.h"
#include "Pos.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>
#include <string>
#include <iostream>

class GameController {
public:
  GameController();
  ~GameController();
  void run();                              // REPL: read commands + dispatch
private:
  std::shared_ptr<Board> board;
  bool gameInProgress;
  double whiteScore = 0.0;
  double blackScore = 0.0;
  
  // X11 Graphics members
  Display* display = nullptr;
  Window window = 0;
  GC gc = nullptr;
  int cellSize = 70;
  bool graphicsActive = false;
  
  // Colors
  unsigned long whiteSquareColor;
  unsigned long blackSquareColor;
  unsigned long whitePieceColor;
  unsigned long blackPieceColor;
  unsigned long textColor;
  
  // Graphics methods
  bool initGraphics();
  void renderGraphics() const;
  bool processGraphicsEvents();
  void closeGraphics();
  void drawPieceSprite(int x, int y, char symbol) const;
  
  // Game logic methods
  void printFinalScore() const;
  void printScore() const;
  void incrementScore(Colour winner);
  void incrementDrawScore();
  bool processCommand(const std::string& cmd);
  Pos parsePos(const std::string& pos);
};

#endif // GAME_CONTROLLER_H 