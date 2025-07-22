# Chess246

A command-line chess game implemented in C++20 for CS246 Spring 2025.

## Features
- Text-only UI with ASCII board representation
- Human vs Human gameplay
- Basic move legality for all six piece types
- Pawn auto-promotion to Queen
- No castling or en-passant

## Supported Commands
- `game human human` - Start a new game
- `move <src> <dst>` - Move a piece (e.g., `move e2 e4`)
- `resign` - Resign the current game
- Ctrl-D to quit

## Building
```
make
```

## Running
```
./chess
```