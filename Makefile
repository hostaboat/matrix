PROJ = Matrix
CC = g++
CFLAGS = -std=c++11 -Wall -I .
DEBUGFLAGS = -g -O0
LDLIBS = -lncurses

SOURCES = \
Number.H \
Number.C \
Rational.C \
Rational.H \
Scientific.C \
Scientific.H \
Matrix.C \
Matrix.H \
MatrixEditor.C \
MatrixEditor.H \
Exceptions.H \
Exceptions.C \
main.C

OBJECTS = \
Rational.o \
Scientific.o \
MatrixEditor.o \
Exceptions.o \
main.o

all: $(PROJ)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(PROJ)

$(PROJ): $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(PROJ)

-include $(OBJECTS:.o=.d)

%.o: %.C
	$(CC) $(CFLAGS) -c $*.C -o $*.o
	@$(CC) -MM $(CFLAGS) $*.C > $*.d

################################################################################

# Utility for printing the code you have written for the project.  
# Typing 'make print' produces a PostScript file named $(PROJ).ps 
# to be printed on an appropriate PS printer such as acsps.
PRINTPGM   = a2ps
PRINTFLAGS = --pretty-print=cxx -g
PRINTFILE  = $(PROJ).ps
.PHONY: print
print: $(SOURCES)
	- $(PRINTPGM) $(PRINTFLAGS) $(SOURCES) Makefile -o $(PRINTFILE)

# Utility for printing all the code -- both the code you have written 
# and the code that was provided for the project.  
# Typing 'make printall' produces a PostScript file named $(PROJ).ps 
# to be printed on an appropriate PS printer such as acsps.
.PHONY: printall
printall: $(SOURCES) $(PROVIDED_SOURCES)
	- $(PRINTPGM) $(PRINTFLAGS) \
              $(SOURCES) $(PROVIDED_SOURCES) Makefile -o $(PRINTFILE)

# Utilities for cleaning up your directory.  
# 'make clean' removes emacs backup files
# 'make cleaner' also removes all object files
# 'make cleanest' also removes core, the executable, and
#      the ii_files directory created by the SGI compiler
.PHONY: clean
#.PHONY: cleaner
#.PHONY: cleanest
#clean:
#	- rm -f *# *~ 
#cleaner: clean
#	- rm -f *.o
#cleanest: cleaner
#	- rm -f core; rm -f $(PROJ); rm -rf ii_files
clean:
	- rm -rf *.o *.d $(PROJ)
