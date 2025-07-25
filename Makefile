CXX = g++-14
CXXFLAGS = -std=c++20 -fmodules-ts -Wall -Wextra
X11FLAGS = -lX11

# Original source files
SOURCES = Piece.cc Pawn.cc Knight.cc Bishop.cc Rook.cc Queen.cc King.cc Board.cc GameController.cc main.cc
HEADERS = Colour.h Pos.h Piece.h Pawn.h Knight.h Bishop.h Rook.h Queen.h King.h Board.h GameController.h
OBJECTS = $(SOURCES:.cc=.o)

.PHONY: all clean

all: chess

chess: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(X11FLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o chess 