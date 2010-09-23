#
# Makefile for the grammar module
#

.PHONY : clean all lib check 

# Module name
MODULE = grammar

# Module Common name
MODULE_COMMON = common

# Common module location
COMMON = ../common

# Module ContentIO name
MODULE_CONTENT_IO = contentIO

# Common ContentIO location
CONTENT_IO = ../contentIO

# Module StreamIO name
MODULE_STREAM_IO = streamIO

# Common StreamIO location
STREAM_IO = ../streamIO

# Module StringTables name
MODULE_STRING_TABLES = stringTables

# Common StringTables location
STRING_TABLES = ../stringTables

# Specify compiler to be used
CC = gcc

# Common compiler flags
CFLAGS = -I$(COMMON)/include -I$(CONTENT_IO)/include/

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

all: lib

lib: lib$(MODULE).a lib$(MODULE_COMMON).a lib$(MODULE_STREAM_IO).a lib$(MODULE_STRING_TABLES).a

check: test
		./test

test: lib tests/check_grammar.c
		$(CC) $(CFLAGS) tests/check_grammar.c lib$(MODULE).a lib$(MODULE_STREAM_IO).a \
        lib$(MODULE_COMMON).a $(CHECK_DIR)/libcheck.a lib$(MODULE_STRING_TABLES).a -o test

clean:
		rm -f *.o *.a test

lib$(MODULE).a: eventsEXI.o grammarRules.o grammars.o
		ar rcs lib$(MODULE).a eventsEXI.o grammarRules.o grammars.o
		
eventsEXI.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/eventsEXI.h src/eventsEXI.c
		$(CC) -c $(CFLAGS) src/eventsEXI.c

grammarRules.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/eventsEXI.h include/grammarRules.h src/grammarRules.c
		$(CC) -c $(CFLAGS) src/grammarRules.c
		
grammars.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h include/eventsEXI.h include/grammarRules.h src/grammars.c \
             $(CONTENT_IO)/include/contentHandler.h
		$(CC) -c $(CFLAGS) src/grammars.c
		
		
		
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
		

# String Tables module build - TODO: reuse the String Tables module make file
lib$(MODULE_STRING_TABLES).a: sTables.o
		ar rcs lib$(MODULE_STRING_TABLES).a sTables.o
		
sTables.o : $(COMMON)/include/errorHandle.h $(COMMON)/include/procTypes.h \
            $(COMMON)/include/stringManipulate.h $(STRING_TABLES)/include/sTables.h $(STRING_TABLES)/src/sTables.c
		$(CC) -c $(CFLAGS) $(STRING_TABLES)/src/sTables.c
		