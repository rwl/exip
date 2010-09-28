#
# Makefile for the contentIO module
#

.PHONY : clean all lib check 

# Module name
MODULE = contentIO

# Module Common name
MODULE_COMMON = common

# Common module location
COMMON = ../common

# Module StreamIO name
MODULE_STREAM_IO = streamIO

# Common StreamIO location
STREAM_IO = ../streamIO

# Specify compiler to be used
CC = gcc

# Common compiler flags
CFLAGS = -I$(COMMON)/include

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

all: lib

lib: lib$(MODULE).a lib$(MODULE_COMMON).a lib$(MODULE_STREAM_IO).a

check: test
		./test

test: lib tests/check_contentIO.c
		$(CC) $(CFLAGS) tests/check_contentIO.c lib$(MODULE).a lib$(MODULE_COMMON).a lib$(MODULE_STREAM_IO).a $(CHECK_DIR)/libcheck.a -o test

clean:
		rm -f *.o *.a test

lib$(MODULE).a: headerDecode.o headerEncode.o
		ar rcs lib$(MODULE).a headerDecode.o headerEncode.o
		
headerEncode.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(COMMON)/include/stringManipulate.h include/headerEncode.h src/headerEncode.c
		$(CC) -c $(CFLAGS) src/headerEncode.c
		
headerDecode.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(COMMON)/include/stringManipulate.h include/headerDecode.h src/headerDecode.c
		$(CC) -c $(CFLAGS) src/headerDecode.c		

# Common module build - TODO: make it separate build file
lib$(MODULE_COMMON).a : p_ASCII_stringManipulate.o procTypes.o
		ar rcs lib$(MODULE_COMMON).a p_ASCII_stringManipulate.o procTypes.o

p_ASCII_stringManipulate.o : $(COMMON)/src/p_ASCII_stringManipulate.c $(COMMON)/include/stringManipulate.h $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/p_ASCII_stringManipulate.c
	
procTypes.o : $(COMMON)/src/procTypes.c $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/procTypes.c
		
# StreamIO module build - TODO: reuse the StreamIO module make file
lib$(MODULE_STREAM_IO).a: streamDecode.o streamRead.o streamWrite.o
		ar rcs lib$(MODULE_STREAM_IO).a streamDecode.o streamRead.o streamWrite.o
		
streamDecode.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(STREAM_IO)/include/streamDecode.h $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamDecode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamDecode.c

streamRead.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamRead.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamRead.c
		
streamWrite.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h $(STREAM_IO)/include/streamWrite.h $(STREAM_IO)/src/streamWrite.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamWrite.c		