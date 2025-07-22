#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "Board.h"
#include "Pos.h"
#include <memory>
#include <string>
#include <iostream>

class GameController {
public:
  GameController();
  void run();                              // REPL: read commands + dispatch
private:
  std::shared_ptr<Board> board;
  bool gameInProgress;
  bool processCommand(const std::string& cmd);
  Pos parsePos(const std::string& pos);
};

#endif // GAME_CONTROLLER_H 