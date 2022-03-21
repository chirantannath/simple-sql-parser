CC = gcc
CXX = g++
COMMONFLAGS = -std=c++17
DEBUGFLAGS = $(COMMONFLAGS) -DDEBUG -g -Og
RELEASEFLAGS = $(COMMONFLAGS) -O2 -s
TARGET = simple-sql-parser.out

#Change this in the makefile when compiling for release.
FLAGS = $(DEBUGFLAGS)

all: $(TARGET)

$(TARGET): main.o lexer.o
	$(CXX) $(FLAGS) -o $@ $+

main.o: main.cpp lexer.hpp
	$(CXX) $(FLAGS) -o $@ -c $<

lexer.o: lexer.cpp lexer.hpp
	$(CXX) $(FLAGS) -o $@ -c $<

clean:
	rm -f *.o

all-clean: clean
	rm -f $(TARGET)