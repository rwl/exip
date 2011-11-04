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

target ?= pc

params ?= $(target)/build-params.mk

include $(params)

CC = $(COMPILER)

CFLAGS += $(WARNING)
CFLAGS += $(DEBUGGING)
CFLAGS += $(ADDITIONAL_CFLAGS)

# The project root directory
PROJECT_ROOT = ../..

# Check Unit Test installation directory
CHECK_DIR = $(CHECK_UNIT_TEST_DIR)

# The src folder of the project 
SOURCE_DIR = $(PROJECT_ROOT)/src

# The bin folder of the project 
BIN_DIR = $(PROJECT_ROOT)/bin

# The documentation folder of the project
DOC_DIR = $(PROJECT_ROOT)/doc

# Common module
COMMON = common
COMMON_SRC = $(SOURCE_DIR)/$(COMMON)

# StreamIO module
STREAM_IO = streamIO
STREAM_IO_SRC = $(SOURCE_DIR)/$(STREAM_IO)

# String Tables module
STRING_TABLES = stringTables
STRING_TABLES_SRC = $(SOURCE_DIR)/$(STRING_TABLES)

# Grammar module
GRAMMAR = grammar
GRAMMAR_SRC = $(SOURCE_DIR)/$(GRAMMAR)

# ContentIO module
CONTENT_IO = contentIO
CONTENT_IO_SRC = $(SOURCE_DIR)/$(CONTENT_IO)

# GrammarGen module
GRAMMAR_GEN = grammarGen
GRAMMAR_GEN_SRC = $(SOURCE_DIR)/$(GRAMMAR_GEN)

# Unit tests src root directory
TESTS_SRC_DIR = $(PROJECT_ROOT)/tests

# Unit tests bin directory
TESTS_BIN_DIR = $(BIN_DIR)/tests

# Public include directory
PUBLIC_INCLUDE_DIR = $(PROJECT_ROOT)/include

# Compiler include flags
CFLAGS += -I$(COMMON_SRC)/include
CFLAGS += -I$(GRAMMAR_SRC)/include
CFLAGS += -I$(STRING_TABLES_SRC)/include
CFLAGS += -I$(CONTENT_IO_SRC)/include
CFLAGS += -I$(STREAM_IO_SRC)/include
CFLAGS += -I$(GRAMMAR_GEN_SRC)/include
CFLAGS += -I$(PUBLIC_INCLUDE_DIR)
CFLAGS += -I$(target)

LDFLAGS += -L$(BIN_DIR) 

# Gets the first goal set on the command line - it is used if the next goal is check. Defines which module to test
FIRST_GOAL = $(word 1,$(MAKECMDGOALS))

# If the first goal is check then the target for checking is the whole project
ifeq ($(FIRST_GOAL), check)
   CHECK_TARGET=$(TESTS_BIN_DIR)/test_streamIO $(TESTS_BIN_DIR)/test_stringTables $(TESTS_BIN_DIR)/test_grammar $(TESTS_BIN_DIR)/test_contentIO $(TESTS_BIN_DIR)/test_exip
else
   CHECK_TARGET=$(TESTS_BIN_DIR)/test_$(FIRST_GOAL)
endif

# List of all example binary names
EXAMPLE_1 = exipd

EXAMPLE_2 = exipe

# Examples directories
EXAMPLES_SRC_DIR = $(PROJECT_ROOT)/examples
EXAMPLES_BIN_DIR = $(BIN_DIR)/examples

# List of all utils binary names
UTILS_1 = exipg

# Utils directories
UTILS_SRC_DIR = $(PROJECT_ROOT)/utils
UTILS_BIN_DIR = $(BIN_DIR)/utils

include common.mk

include streamIO.mk

include stringTables.mk

include grammar.mk

include contentIO.mk

include grammarGen.mk

include tests.mk

include examples.mk

include utils.mk

OBJECT_ALL = $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ)

ifeq ($(INCLUDE_GRAMMAR_GENERATION), true)
   OBJECT_ALL += $(GRAMMAR_GEN_OBJ)
endif

.PHONY : clean all lib check examples exipd mk_bin mk_examples \
		 mk_tests mk_utils test_sets_copy_examples utils test_sets_copy_utils doc

all: mk_bin $(OBJECT_ALL)

lib: mk_bin $(BIN_DIR)/libexip.a

$(BIN_DIR)/libexip.a: $(OBJECT_ALL)
		ar rcs $(BIN_DIR)/libexip.a $(OBJECT_ALL)
			
check: mk_tests $(CHECK_TARGET)
		for i in $(CHECK_TARGET); do \
                 $$i; \
             done

examples: mk_examples lib $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1) $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2) test_sets_copy_examples

utils: mk_utils lib $(UTILS_BIN_DIR)/$(UTILS_1) test_sets_copy_utils

doc:
		cd $(DOC_DIR); doxygen Doxyfile

mk_bin:
		mkdir -p $(BIN_DIR)
		
mk_examples:
		mkdir -p $(EXAMPLES_BIN_DIR)
		
mk_utils:
		mkdir -p $(UTILS_BIN_DIR)
		
mk_tests:
		mkdir -p $(TESTS_BIN_DIR)
		
test_sets_copy_examples:
		cp $(EXAMPLES_SRC_DIR)/simpleEncoding/exip-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipe-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipd-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test.exi $(EXAMPLES_BIN_DIR)/exipd-test.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exip-test-schema.exi $(EXAMPLES_BIN_DIR)/exipd-test-schema.exi
		
test_sets_copy_utils:		
		cp $(UTILS_SRC_DIR)/schemaHandling/EXIOptions-xsd.exi $(UTILS_BIN_DIR)/EXIOptions-xsd.exi
		
clean:
		rm -f -r $(BIN_DIR)