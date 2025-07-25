CXX = g++-14
CXXFLAGS = -std=c++20 -fmodules-ts -Wall -Wextra
X11FLAGS = -lX11

SOURCES = Piece.cc Pawn.cc Knight.cc Bishop.cc Rook.cc Queen.cc King.cc Board.cc ChessGraphics.cc ChessMain.cc
HEADERS = Colour.h Pos.h Piece.h Pawn.h Knight.h Bishop.h Rook.h Queen.h King.h Board.h ChessGraphics.h
OBJECTS = $(SOURCES:.cc=.o)

.PHONY: all clean

all: chess

chess: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(X11FLAGS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# X11 test program
x11test: x11test.cc
	$(CXX) $(CXXFLAGS) x11test.cc -o x11test $(X11FLAGS)

clean:
	rm -f *.o chess x11test 