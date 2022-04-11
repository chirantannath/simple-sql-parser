CC = gcc
CXX = g++
COMMONFLAGS = -std=c++17
DEBUGFLAGS = $(COMMONFLAGS) -DDEBUG -g
RELEASEFLAGS = $(COMMONFLAGS) -O2 -s
TARGET = simple-sql-parser.out
HEADERS = error.hpp lexer.hpp parser.hpp setutil.hpp parsegen1.hpp parsegen2.hpp

#Change this in the makefile when compiling for release; or
#make 'FLAGS=$(RELEASEFLAGS)' <other parameters>
FLAGS = $(DEBUGFLAGS)

all: $(TARGET)

$(TARGET): main.o error.o lexer.o parser.o cfg.o setutil.o parsegen1.o parsegen2.o
	$(CXX) $(FLAGS) -o $@ $+

main.o: main.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

lexer.o: lexer.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

error.o: error.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

parser.o: parser.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

cfg.o: cfg.cpp ${HEADERS}
	$(CXX) $(FLAGS) -o $@ -c $<

setutil.o: setutil.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

parsegen1.o: parsegen1.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

parsegen2.o: parsegen2.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

clean:
	rm -fv *.o

all-clean: clean
	rm -fv $(TARGET)