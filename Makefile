CC = cc
CXX = c++
COMMONFLAGS = -std=c++17 -Wall -Wextra
DEBUGFLAGS = $(COMMONFLAGS) -Werror -DDEBUG -g 
TESTFLAGS = $(COMMONFLAGS) -g
#Test flags is debug flags without the debug. Used for testing for release.
RELEASEFLAGS = $(COMMONFLAGS) -O2 -s
PROFILEFLAGS = $(COMMONFLAGS) -pg
#Profile flags used for profiling (performance measurement)
TARGET = simple-sql-parser.out
HEADERS = error.hpp lexer.hpp parser.hpp setutil.hpp parsegen1.hpp parsegen2.hpp parsegen3.hpp setutil.cpp
#setutil.cpp acts as a header because it is filled with template definitions. 

#Change this in the makefile when checking for debug; or
#make 'FLAGS=$(DEBUGFLAGS)' <other parameters>
FLAGS = $(RELEASEFLAGS)

all: $(TARGET)

$(TARGET): main.o error.o lexer.o parser.o cfg.o setutil.o parsegen1.o parsegen2.o parsegen3.o
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

parsegen3.o: parsegen3.cpp $(HEADERS)
	$(CXX) $(FLAGS) -o $@ -c $<

clean:
	rm -fv *.o

all-clean: clean
	rm -fv $(TARGET)