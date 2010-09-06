#
# Makefile for the streamIO module
#

.PHONY : clean all lib check 

# Module name
MODULE = streamIO

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

test: lib tests/check_streamIO.c
		$(CC) $(CFLAGS) tests/check_streamIO.c lib$(MODULE).a lib$(MODULE_COMMON).a $(CHECK_DIR)/libcheck.a -o test

clean:
		rm -f *.o *.a test

lib$(MODULE).a: streamDecode.o streamRead.o streamWrite.o
		ar rcs lib$(MODULE).a streamDecode.o streamRead.o streamWrite.o
		
streamDecode.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/streamDecode.h include/streamRead.h src/streamDecode.c
		$(CC) -c $(CFLAGS) src/streamDecode.c

streamRead.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/streamRead.h src/streamRead.c
		$(CC) -c $(CFLAGS) src/streamRead.c
		
streamWrite.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/streamWrite.h src/streamWrite.c
		$(CC) -c $(CFLAGS) src/streamWrite.c
		
		
		
# Common module build - TODO: make it separate build file
lib$(MODULE_COMMON).a : p_ASCII_stringManipulate.o procTypes.o
		ar rcs lib$(MODULE_COMMON).a p_ASCII_stringManipulate.o procTypes.o

p_ASCII_stringManipulate.o : $(COMMON)/src/p_ASCII_stringManipulate.c $(COMMON)/include/stringManipulate.h $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/p_ASCII_stringManipulate.c
	
procTypes.o : $(COMMON)/src/procTypes.c $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/procTypes.c