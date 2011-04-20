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

CFLAGS += -Wpacked -Wall

CFLAGS += -O0 -g # Debugging 

# The project root directory
PROJECT_ROOT = ../..

# Check Unit Test installation directory
CHECK_DIR = /usr/local/lib

# The src folder of the project 
SOURCE_DIR = $(PROJECT_ROOT)/src

# The bin folder of the project 
BIN_DIR = bin

# Common module
COMMON = common
COMMON_SRC = $(SOURCE_DIR)/$(COMMON)
COMMON_BIN = $(BIN_DIR)/$(COMMON)

# StreamIO module
STREAM_IO = streamIO
STREAM_IO_SRC = $(SOURCE_DIR)/$(STREAM_IO)
STREAM_IO_BIN = $(BIN_DIR)/$(STREAM_IO)

# String Tables module
STRING_TABLES = stringTables
STRING_TABLES_SRC = $(SOURCE_DIR)/$(STRING_TABLES)
STRING_TABLES_BIN = $(BIN_DIR)/$(STRING_TABLES)

# Grammar module
GRAMMAR = grammar
GRAMMAR_SRC = $(SOURCE_DIR)/$(GRAMMAR)
GRAMMAR_BIN = $(BIN_DIR)/$(GRAMMAR)

# ContentIO module
CONTENT_IO = contentIO
CONTENT_IO_SRC = $(SOURCE_DIR)/$(CONTENT_IO)
CONTENT_IO_BIN = $(BIN_DIR)/$(CONTENT_IO)

# GrammarGen module
GRAMMAR_GEN = grammarGen
GRAMMAR_GEN_SRC = $(SOURCE_DIR)/$(GRAMMAR_GEN)
GRAMMAR_GEN_BIN = $(BIN_DIR)/$(GRAMMAR_GEN)

# Unit tests src root directory
TESTS_SRC_DIR = $(PROJECT_ROOT)/tests

# Unit tests bin directory
TESTS_BIN_DIR = $(BIN_DIR)/tests

# Public include directory
PUBLIC_INCLUDE_DIR = $(PROJECT_ROOT)/include

# Common compiler flags
CFLAGS += -I$(COMMON_SRC)/include
CFLAGS += -I$(GRAMMAR_SRC)/include
CFLAGS += -I$(STRING_TABLES_SRC)/include
CFLAGS += -I$(CONTENT_IO_SRC)/include
CFLAGS += -I$(STREAM_IO_SRC)/include
CFLAGS += -I$(GRAMMAR_GEN_SRC)/include
CFLAGS += -I$(PUBLIC_INCLUDE_DIR)

# Gets the first goal set on the command line - it is used if the next goal is check. Defines which module to test
TARGET = $(word 1,$(MAKECMDGOALS))

# If the first goal is check then the target for checking is the whole project
ifeq ($(TARGET), check)
   CHECK_TARGET=$(TESTS_BIN_DIR)/test_streamIO $(TESTS_BIN_DIR)/test_stringTables $(TESTS_BIN_DIR)/test_grammar $(TESTS_BIN_DIR)/test_contentIO
else
   CHECK_TARGET=$(TESTS_BIN_DIR)/test_$(TARGET)
endif

# List of all example binary names
EXAMPLE_1 = exipd

EXAMPLE_2 = exipe

# Examples directories
EXAMPLES_SRC_DIR = $(PROJECT_ROOT)/examples
EXAMPLES_BIN_DIR = $(BIN_DIR)/examples

all: lib

include common.mk

include streamIO.mk

include stringTables.mk

include grammar.mk

include contentIO.mk

include grammarGen.mk

include tests.mk

OBJECT_ALL = $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(GRAMMAR_GEN_OBJ)

.PHONY : clean all lib check common streamIO stringTables grammar contentIO grammarGen examples exipd \
         mk_bin mk_common mk_streamIO mk_stringTables mk_grammar mk_contentIO mk_grammarGen mk_examples mk_tests mk_all \
         test_sets_copy
         
lib: mk_bin $(BIN_DIR)/libexip.a

$(BIN_DIR)/libexip.a: mk_all $(OBJECT_ALL)
		ar rcs $(BIN_DIR)/libexip.a $(OBJECT_ALL)
		
common: mk_common $(COMMON_BIN)/lib$(COMMON).a
		
streamIO: mk_streamIO common $(STREAM_IO_BIN)/lib$(STREAM_IO).a		
		
stringTables: mk_stringTables common $(STRING_TABLES_BIN)/lib$(STRING_TABLES).a		

grammar: mk_grammar common streamIO stringTables $(GRAMMAR_BIN)/lib$(GRAMMAR).a

contentIO: mk_contentIO common streamIO $(CONTENT_IO_BIN)/lib$(CONTENT_IO).a

grammarGen: mk_grammarGen common stringTables $(GRAMMAR_GEN_BIN)/lib$(GRAMMAR_GEN).a
		
check: mk_tests $(CHECK_TARGET)
		for i in $(CHECK_TARGET); do \
                 $$i; \
             done
		
$(EXAMPLES_BIN_DIR)/$(EXAMPLE_1): $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(BIN_DIR)/libexip.a
		$(CC) $(CFLAGS) $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(BIN_DIR)/libexip.a -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1)
		
$(EXAMPLES_BIN_DIR)/$(EXAMPLE_2): $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(BIN_DIR)/libexip.a
		$(CC) $(CFLAGS) $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(BIN_DIR)/libexip.a -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2)		

examples: mk_examples lib $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1) $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2) test_sets_copy

mk_bin:
		mkdir -p $(BIN_DIR)
		
mk_common:		
		mkdir -p $(COMMON_BIN)
		
mk_streamIO:
		mkdir -p $(STREAM_IO_BIN)
		
mk_stringTables:
		mkdir -p $(STRING_TABLES_BIN)
		
mk_grammar:
		mkdir -p $(GRAMMAR_BIN)
		
mk_contentIO:
		mkdir -p $(CONTENT_IO_BIN)

mk_grammarGen:
		mkdir -p $(GRAMMAR_GEN_BIN)
		
mk_examples:
		mkdir -p $(EXAMPLES_BIN_DIR)
		
mk_tests:
		mkdir -p $(TESTS_BIN_DIR)
		
test_sets_copy:
		cp $(EXAMPLES_SRC_DIR)/simpleEncoding/exip-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipe-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipd-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test.exi $(EXAMPLES_BIN_DIR)/exipd-test.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test-schema.exi $(EXAMPLES_BIN_DIR)/exipd-test-schema.exi
		
mk_all: mk_common mk_streamIO mk_stringTables mk_grammar mk_contentIO mk_grammarGen
	
clean:
		rm -f -r $(BIN_DIR)