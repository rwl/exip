#/*==================================================================================*\
#|                                                                                    |
#|                    EXIP - Efficient XML Interchange Processor                      |
#|                                                                                    |
#|------------------------------------------------------------------------------------|
#| Copyright (c) 2010, EISLAB - Luleå University of Technology                        |
#| All rights reserved.                                                               |
#|                                                                                    |
#| Redistribution and use in source and binary forms, with or without                 |
#| modification, are permitted provided that the following conditions are met:        |
#|     * Redistributions of source code must retain the above copyright               |
#|       notice, this list of conditions and the following disclaimer.                |
#|     * Redistributions in binary form must reproduce the above copyright            |
#|       notice, this list of conditions and the following disclaimer in the          |
#|       documentation and/or other materials provided with the distribution.         |
#|     * Neither the name of the EISLAB - Luleå University of Technology nor the      |
#|       names of its contributors may be used to endorse or promote products         |
#|       derived from this software without specific prior written permission.        |
#|                                                                                    |
#| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    |
#| ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      |
#| WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             |
#| DISCLAIMED. IN NO EVENT SHALL EISLAB - LULEÅ UNIVERSITY OF TECHNOLOGY BE LIABLE    |
#| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES |
#| (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       |
#| LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND        |
#| ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT         |
#| (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      |
#| SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       |
#|                                                                                    |
#|                                                                                    |
#|                                                                                    |
#\===================================================================================*/
#
#/**
# * Makefile for EXIP library
# * @date Oct 8, 2010
# * @author Rumen Kyusakov
# * @version 0.1
# * @par[Revision] $Id$
# */

# Specify compiler to be used
CC = gcc

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

# Common module
COMMON = common

# StreamIO module
STREAM_IO = streamIO

# String Tables module
STRING_TABLES = stringTables

# Grammar module
GRAMMAR = grammar

# ContentIO module
CONTENT_IO = contentIO

# Common compiler flags
CFLAGS = -I$(COMMON)/include
CFLAGS += -I$(GRAMMAR)/include
CFLAGS += -I$(STRING_TABLES)/include
CFLAGS += -I$(CONTENT_IO)/include

CFLAGS += -g # Debugging 

# Gets the first goal set on the command line - it is used if the next goal is check. Defines which module to test
TARGET = $(word 1,$(MAKECMDGOALS))

# List of all example binary names
EXAMPLE_1 = exipd

EXAMPLE_2 = exipe

# Examples directory
EXAMPLES_DIR = examples

all: lib

include $(COMMON)/module.mk

include $(STREAM_IO)/module.mk

include $(CONTENT_IO)/module.mk

include $(STRING_TABLES)/module.mk

include $(GRAMMAR)/module.mk

OBJECT_ALL = $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ)

.PHONY : clean all lib check common streamIO stringTables examples exipd

lib: exip.a

exip.a: $(OBJECT_ALL)
		ar rcs exip.a $(OBJECT_ALL)
		
common: $(COMMON)/lib$(COMMON).a
		
streamIO: common $(STREAM_IO)/lib$(STREAM_IO).a		
		
stringTables: common $(STRING_TABLES)/lib$(STRING_TABLES).a		

grammar: common streamIO stringTables $(GRAMMAR)/lib$(GRAMMAR).a

contentIO: common streamIO $(CONTENT_IO)/lib$(CONTENT_IO).a
		
check: $(TARGET)/test
		$(TARGET)/test
		
$(EXAMPLE_1): $(EXAMPLES_DIR)/simpleDecoding/decodeTestEXI.c exip.a
		$(CC) $(CFLAGS) $(EXAMPLES_DIR)/simpleDecoding/decodeTestEXI.c exip.a -o $(EXAMPLES_DIR)/$(EXAMPLE_1)
		
$(EXAMPLE_2): $(EXAMPLES_DIR)/simpleEncoding/encodeTestEXI.c exip.a
		$(CC) $(CFLAGS) $(EXAMPLES_DIR)/simpleEncoding/encodeTestEXI.c exip.a -o $(EXAMPLES_DIR)/$(EXAMPLE_2)		

examples: lib $(EXAMPLE_1) $(EXAMPLE_2)
	
clean:
		rm -f *.o *.a test
		rm -f $(COMMON)/*.o $(COMMON)/*.a $(COMMON)/test
		rm -f $(STREAM_IO)/*.o $(STREAM_IO)/*.a $(STREAM_IO)/test
		rm -f $(STRING_TABLES)/*.o $(STRING_TABLES)/*.a $(STRING_TABLES)/test
		rm -f $(GRAMMAR)/*.o $(GRAMMAR)/*.a $(GRAMMAR)/test
		rm -f $(CONTENT_IO)/*.o $(CONTENT_IO)/*.a $(CONTENT_IO)/test
		rm -f $(EXAMPLES_DIR)/*.o $(EXAMPLES_DIR)/*.a $(EXAMPLES_DIR)/$(EXAMPLE_1) $(EXAMPLES_DIR)/$(EXAMPLE_2)