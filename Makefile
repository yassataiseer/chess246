CXX = g++-14
CXXFLAGS = -std=c++20 -fmodules-ts -Wall -Wextra -Werror

MODULES = Colour.cc Pos.cc Piece.cc Pawn.cc Knight.cc Bishop.cc Rook.cc Queen.cc King.cc Board.cc GameController.cc
IMPLS = Colour-impl.cc Pos-impl.cc Piece-impl.cc Pawn-impl.cc Knight-impl.cc Bishop-impl.cc Rook-impl.cc Queen-impl.cc King-impl.cc Board-impl.cc GameController-impl.cc
MAIN = main.cc

PCMS = $(MODULES:.cc=.pcm)
OBJS = $(IMPLS:.cc=.o) main.o

.PHONY: all clean

all: chess

chess: $(PCMS) $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.pcm: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%-impl.o: %-impl.cc %.pcm
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o: $(MAIN) $(PCMS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o *.pcm chess 