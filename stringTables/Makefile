#
# Makefile for the String Tables module
#

.PHONY : clean all lib check 

# Module name
MODULE = stringTables

# Module Common name
MODULE_COMMON = common

# Common module location
COMMON = ../common

# Specify compiler to be used
CC = gcc

# Common compiler flags
CFLAGS = -I$(COMMON)/include

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

all: lib

lib: lib$(MODULE).a lib$(MODULE_COMMON).a

check: test
		./test

test: lib tests/check_stringTables.c
		$(CC) $(CFLAGS) tests/check_stringTables.c lib$(MODULE).a lib$(MODULE_COMMON).a $(CHECK_DIR)/libcheck.a -o test

clean:
		rm -f *.o *.a test

lib$(MODULE).a: sTables.o
		ar rcs lib$(MODULE).a sTables.o
		
sTables.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(COMMON)/include/stringManipulate.h include/sTables.h src/sTables.c
		$(CC) -c $(CFLAGS) src/sTables.c

# Common module build - TODO: make it separate build file
lib$(MODULE_COMMON).a : p_ASCII_stringManipulate.o procTypes.o
		ar rcs lib$(MODULE_COMMON).a p_ASCII_stringManipulate.o procTypes.o

p_ASCII_stringManipulate.o : $(COMMON)/src/p_ASCII_stringManipulate.c $(COMMON)/include/stringManipulate.h $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/p_ASCII_stringManipulate.c
	
procTypes.o : $(COMMON)/src/procTypes.c $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/procTypes.c