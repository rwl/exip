#
# Makefile for the streamIO module
#

.PHONY : clean all lib check 

# Module name
MODULE = streamIO

# Common module location
COMMON = ../common

# Specify compiler to be used
CC = gcc

# Common compiler flags
CFLAGS = -I$(COMMON)/include

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

all: lib

lib: lib$(MODULE).a

check: test
		./test

test: lib$(MODULE).a tests/check_streamIO.c
		$(CC) $(CFLAGS) tests/check_streamIO.c lib$(MODULE).a $(CHECK_DIR)/libcheck.a -o test

clean:
		rm -f *.o *.a test

lib$(MODULE).a: streamDecode.o streamRead.o
		ar rcs lib$(MODULE).a streamDecode.o streamRead.o
		
streamDecode.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/streamDecode.h src/streamDecode.c
		$(CC) -c $(CFLAGS) src/streamDecode.c

streamRead.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/streamRead.h src/streamRead.c
		$(CC) -c $(CFLAGS) src/streamRead.c