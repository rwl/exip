#/*==================================================================*\
#|                EXIP - Embeddable EXI Processor in C                |
#|--------------------------------------------------------------------|
#|          This work is licensed under BSD 3-Clause License          |
#|  The full license terms and conditions are located in LICENSE.txt  |
#\===================================================================*/
#
#/**
# * Makefile for the EXIP library
# * @date Oct 8, 2010
# * @author Rumen Kyusakov
# * @version 0.4
# * @par[Revision] $Id$
# */

target ?= pc

params ?= $(target)/build-params.mk

include $(params)

CC = $(COMPILER)

CFLAGS += $(WARNING)
CFLAGS += $(DEBUGGING)
CFLAGS += $(ADDITIONAL_CFLAGS)

# Library version
EXIP_VERSION = 0.4

# The project root directory
PROJECT_ROOT = ../..

# The src folder of the project 
SOURCE_DIR = $(PROJECT_ROOT)/src

# The bin folder of the project 
BIN_DIR = $(PROJECT_ROOT)/bin

# The development documentation folder of the project
DEV_DOC_DIR = $(PROJECT_ROOT)/doc/dev

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

# Unit tests test data directory
TESTS_DATA_DIR = $(PROJECT_ROOT)/tests/test-set

# Unit tests bin directory
TESTS_BIN_DIR = $(BIN_DIR)/tests

# Static and dynamic library output directory
LIB_BIN_DIR = $(BIN_DIR)/lib

# Public include directory
PUBLIC_INCLUDE_DIR = $(PROJECT_ROOT)/include

# Compiler include flags
INCDIRS += -I$(COMMON_SRC)/include
INCDIRS += -I$(GRAMMAR_SRC)/include
INCDIRS += -I$(STRING_TABLES_SRC)/include
INCDIRS += -I$(CONTENT_IO_SRC)/include
INCDIRS += -I$(STREAM_IO_SRC)/include
INCDIRS += -I$(GRAMMAR_GEN_SRC)/include
INCDIRS += -I$(PUBLIC_INCLUDE_DIR)
INCDIRS += -I$(target)

LDFLAGS += -L$(LIB_BIN_DIR) 

# Used by included Makefiles to compile; hides implementation.
COMPILE = $(CC) $(CFLAGS) $(INCDIRS)

# Tests all targets by default. May override in environment to specify checks to run, for example:
#   $make check CHECK_TARGETS="streamio strict_grammar"
CHECK_TARGETS ?= $(STREAM_IO) $(STRING_TABLES) $(GRAMMAR) $(CONTENT_IO) exip builtin_grammar strict_grammar emptyType
CHECK_BINS := $(foreach acheck, $(CHECK_TARGETS), $(TESTS_BIN_DIR)/test_$(acheck))

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

.PHONY : clean all lib dynlib check examples exipd mk_bin mk_examples mk_lib \
		 mk_tests mk_utils test_sets_copy_examples utils test_sets_copy_utils doc dist

all: mk_bin $(OBJECT_ALL)

lib: mk_bin mk_lib $(LIB_BIN_DIR)/libexip.a

$(LIB_BIN_DIR)/libexip.a: $(OBJECT_ALL)
		ar rcs $(LIB_BIN_DIR)/libexip.a $(OBJECT_ALL)

dynlib: COMPILE += -fPIC
dynlib: clean all mk_lib $(LIB_BIN_DIR)/libexip.so

$(LIB_BIN_DIR)/libexip.so: $(OBJECT_ALL)
		$(CC) -shared -Wl,-soname,libexip.so.$(EXIP_VERSION) -o $(LIB_BIN_DIR)/libexip.so.$(EXIP_VERSION) $(OBJECT_ALL)
		ln -sf $(LIB_BIN_DIR)/libexip.so.$(EXIP_VERSION) $(LIB_BIN_DIR)/libexip.so

check: all mk_tests $(CHECK_BINS)
		for i in $(CHECK_BINS); do \
                   $$i $(TESTS_DATA_DIR); \
             done

examples: mk_examples lib $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1) $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2) test_sets_copy_examples

utils: mk_utils lib $(UTILS_BIN_DIR)/$(UTILS_1) test_sets_copy_utils

doc:
		cd $(DEV_DOC_DIR); doxygen Doxyfile

mk_bin:
		mkdir -p $(BIN_DIR)
		
mk_lib:
		mkdir -p $(LIB_BIN_DIR)
		
mk_examples:
		mkdir -p $(EXAMPLES_BIN_DIR)
		
mk_utils:
		mkdir -p $(UTILS_BIN_DIR)
		
mk_tests:
		mkdir -p $(TESTS_BIN_DIR)

test_sets_copy_examples:
		cp $(EXAMPLES_SRC_DIR)/simpleEncoding/exipe-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipe-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exipd-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipd-test-xsd.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exipd-test.exi $(EXAMPLES_BIN_DIR)/exipd-test.exi
		cp $(EXAMPLES_SRC_DIR)/simpleDecoding/exipd-test-schema.exi $(EXAMPLES_BIN_DIR)/exipd-test-schema.exi
		
test_sets_copy_utils:		
		cp $(UTILS_SRC_DIR)/schemaHandling/EXIOptions-xsd.exi $(UTILS_BIN_DIR)/EXIOptions-xsd.exi
		
clean:
		rm -f -r $(BIN_DIR)

# Builds a library distribution/release		
dist:
		rm -f -r $(PROJECT_ROOT)/../../files/exip-$(EXIP_VERSION)
		svn export $(PROJECT_ROOT) $(PROJECT_ROOT)/../../files/exip-$(EXIP_VERSION)
		cd $(PROJECT_ROOT)/../../files && tar cvf - exip-$(EXIP_VERSION) | gzip > exip-$(EXIP_VERSION).tar.gz
		cd $(PROJECT_ROOT)/../../files && zip -lvr exip-$(EXIP_VERSION).zip exip-$(EXIP_VERSION)		