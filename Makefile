CC = gcc
CXX = g++
COMMONFLAGS = -std=c++17
DEBUGFLAGS = $(COMMONFLAGS) -DDEBUG -g
RELEASEFLAGS = $(COMMONFLAGS) -O2 -s
TARGET = simple-sql-parser.out

#Change this in the makefile when compiling for release; or
#make 'FLAGS=${RELEASEFLAGS}' <other parameters>
FLAGS = $(DEBUGFLAGS)

all: $(TARGET)

$(TARGET): main.o error.o lexer.o
	$(CXX) $(FLAGS) -o $@ $+

main.o: main.cpp lexer.hpp error.hpp
	$(CXX) $(FLAGS) -o $@ -c $<

lexer.o: lexer.cpp lexer.hpp error.hpp
	$(CXX) $(FLAGS) -o $@ -c $<

error.o: error.cpp error.hpp lexer.hpp
	$(CXX) $(FLAGS) -o $@ -c $<

clean:
	rm -fv *.o

all-clean: clean
	rm -fv $(TARGET)