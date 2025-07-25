# Chess246

A command-line chess game implemented in C++20 for CS246 Spring 2025.

## Features
- Text-only UI with ASCII board representation
- Human vs Human gameplay
- Basic move legality for all six piece types
- Pawn promotion to Queen, Rook, Bishop, or Knight (if not promotion given, base case is Queen)
- No castling or en-passant

## Supported Commands
- `game human human` - Start a new game
- `move <src> <dst>` - Move a piece (e.g., `move e2 e4`)
- `move <src> <dst> <promotion>` - Move a pawn with promotion (e.g., `move e7 e8 Q`)
  - Valid promotion pieces: Q (Queen), R (Rook), B (Bishop), N (Knight)
- `resign` - Resign the current game
- `score` - Displays the current score
- `draw` - redraws the board
- Ctrl-D to quit

## Building
```
make
```

## Running
```
./chess
```
