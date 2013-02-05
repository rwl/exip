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

.PHONY : clean all dynlib check examples utils doc dist \
		 test_sets_copy_examples test_sets_copy_utils

TARGET ?= pc

params ?= $(TARGET)/build-params.mk

include $(params)

CC = $(COMPILER)

CFLAGS += $(WARNING)
CFLAGS += $(DEBUGGING)
CFLAGS += $(ADDITIONAL_CFLAGS)

# Library version
EXIP_VERSION = 0.4.1

# The project root directory
PROJECT_ROOT = ../..

# The src folder of the project 
SOURCE_DIR = $(PROJECT_ROOT)/src

# The bin folder of the project 
BIN_DIR = $(PROJECT_ROOT)/bin

# The development documentation folder of the project
DEV_DOC_DIR = $(PROJECT_ROOT)/doc/dev

# Unit tests src root directory
TESTS_SRC_DIR = $(PROJECT_ROOT)/tests

# Unit tests test data directory
TESTS_DATA_DIR = $(PROJECT_ROOT)/tests/test-set

# Unit tests bin directory
TESTS_BIN_DIR = $(BIN_DIR)/tests

# Static and dynamic library output directory
LIB_BIN_DIR = $(BIN_DIR)/lib

# Examples output directory
EXAMPLES_BIN_DIR = $(BIN_DIR)/examples

# Utils output directory
UTILS_BIN_DIR = $(BIN_DIR)/utils

# Public include directory
PUBLIC_INCLUDE_DIR = $(PROJECT_ROOT)/include

# All directories containing *.c and *.h files
VPATH = $(PROJECT_ROOT)/src/common/src
VPATH += $(PROJECT_ROOT)/src/contentIO/src
VPATH += $(PROJECT_ROOT)/src/grammar/src
VPATH += $(PROJECT_ROOT)/src/grammarGen/src
VPATH += $(PROJECT_ROOT)/src/streamIO/src
VPATH += $(PROJECT_ROOT)/src/stringTables/src
VPATH += $(PROJECT_ROOT)/src/common/include
VPATH += $(PROJECT_ROOT)/src/contentIO/include
VPATH += $(PROJECT_ROOT)/src/grammar/include
VPATH += $(PROJECT_ROOT)/src/grammarGen/include
VPATH += $(PROJECT_ROOT)/src/streamIO/include
VPATH += $(PROJECT_ROOT)/src/stringTables/include
VPATH += $(PUBLIC_INCLUDE_DIR)
VPATH += $(PROJECT_ROOT)/examples/simpleDecoding
VPATH += $(PROJECT_ROOT)/examples/simpleEncoding
VPATH += $(PROJECT_ROOT)/utils/schemaHandling
VPATH += $(PROJECT_ROOT)/utils/schemaHandling/include
VPATH += $(PROJECT_ROOT)/utils/schemaHandling/output
VPATH += $(TESTS_SRC_DIR)
VPATH += $(TARGET)

# Source files for the common module
COMMON_SRC = ASCII_stringManipulate.c\
		contentHandler.c\
		dynamicArray.c\
		hashtable.c\
		memManagement.c\
		procTypes.c

# Source files for the contentIO module
CONTENT_IO_SRC = bodyDecode.c\
		bodyEncode.c\
		EXIParser.c\
		EXISerializer.c\
		headerDecode.c\
		headerEncode.c\
		initSchemaInstance.c\
		staticEXIOptions.c

# Source files for the grammar module		
GRAMMAR_SRC = grammars.c

# Source files for the grammar generation module
GRAMMAR_GEN_SRC = genUtils.c\
		grammarGenerator.c\
		protoGrammars.c\
		treeTableBuild.c\
		treeTableManipulate.c\
		treeTableToGrammars.c
		
STREAM_IO_SRC =	ioUtil.c\
		streamDecode.c\
		streamEncode.c\
		streamRead.c\
		streamWrite.c

# Source files for the string tables module
STRING_TABLES_SRC = sTables.c

TESTS_SRC = check_builtin_grammar.c\
		check_contentIO.c\
		check_emptyType.c\
		check_exip.c\
		check_grammar.c\
		check_streamIO.c\
		check_strict_grammar.c\
		check_stringTables.c

# Source files for the utils
EXIPG_UTIL_SRC = createGrammars.c\
		exipg.c\
		dynOutputUtils.c\
		exipOutputUtils.c\
		staticOutputUtils.c\
		textOutputUtils.c

# Source files for the examples
EXIPE_EXAMPLE_SRC = encodeTestEXI.c exipe.c
EXIPD_EXAMPLE_SRC = decodeTestEXI.c exipd.c

LIB_SOURCES = $(COMMON_SRC) $(CONTENT_IO_SRC) $(GRAMMAR_SRC) $(STREAM_IO_SRC) $(STRING_TABLES_SRC)

ifeq ($(INCLUDE_GRAMMAR_GENERATION), true)
   LIB_SOURCES += $(GRAMMAR_GEN_SRC)
endif

EXIPE_OBJECTS=$(EXIPE_EXAMPLE_SRC:%.c=$(BIN_DIR)/%.o)
EXIPD_OBJECTS=$(EXIPD_EXAMPLE_SRC:%.c=$(BIN_DIR)/%.o)
EXIPG_OBJECTS=$(EXIPG_UTIL_SRC:%.c=$(BIN_DIR)/%.o)
LIB_OBJECTS=$(LIB_SOURCES:%.c=$(BIN_DIR)/%.o)

SOURCES_ALL = $(COMMON_SRC) $(CONTENT_IO_SRC) $(GRAMMAR_SRC) $(STREAM_IO_SRC) $(STRING_TABLES_SRC)\
	$(GRAMMAR_GEN_SRC) $(TESTS_SRC) $(EXIPG_UTIL_SRC) $(EXIPE_EXAMPLE_SRC) $(EXIPD_EXAMPLE_SRC)

# Compiler include flags
INCDIRS += -I$(PROJECT_ROOT)/src/common/include
INCDIRS += -I$(PROJECT_ROOT)/src/contentIO/include
INCDIRS += -I$(PROJECT_ROOT)/src/grammar/include
INCDIRS += -I$(PROJECT_ROOT)/src/grammarGen/include
INCDIRS += -I$(PROJECT_ROOT)/src/streamIO/include
INCDIRS += -I$(PROJECT_ROOT)/src/stringTables/include
INCDIRS += -I$(PUBLIC_INCLUDE_DIR)
INCDIRS += -I$(TARGET)
INCDIRS += -I$(PROJECT_ROOT)/examples/simpleEncoding
INCDIRS += -I$(PROJECT_ROOT)/examples/simpleDecoding
INCDIRS += -I$(PROJECT_ROOT)/tests
INCDIRS += -I$(PROJECT_ROOT)/utils/schemaHandling/include

LDFLAGS += -L$(LIB_BIN_DIR) 

# Tests all targets by default. May override in environment to specify checks to run, for example:
#   $make check CHECK_TARGETS="streamio strict_grammar"
CHECK_TARGETS ?= streamIO stringTables grammar contentIO exip builtin_grammar strict_grammar emptyType
CHECK_BINS := $(foreach acheck, $(CHECK_TARGETS), $(TESTS_BIN_DIR)/test_$(acheck))

EXAMPLE_BINS := $(EXAMPLES_BIN_DIR)/exipd $(EXAMPLES_BIN_DIR)/exipe

UTILS_BINS := $(UTILS_BIN_DIR)/exipg

# Used by included Makefiles to compile; hides implementation.
COMPILE = $(CC) $(CFLAGS) $(INCDIRS)

# TARGET: (DEFAULT) Building the exip library
all: $(BIN_DIR) $(LIB_BIN_DIR) $(LIB_OBJECTS) $(LIB_BIN_DIR)/libexip.a

# TARGET: Building exip as dynamic library
dynlib: COMPILE += -fPIC
dynlib: clean all $(LIB_BIN_DIR)/libexip.so

# TARGET: Exacute the unit tests
check: all $(TESTS_BIN_DIR) $(CHECK_BINS)
		for i in $(CHECK_BINS); do \
                   $$i $(TESTS_DATA_DIR); \
             done

# TARGET: Builds the example applications        
examples: all $(EXAMPLES_BIN_DIR) $(EXAMPLE_BINS) test_sets_copy_examples

# TARGET: Builds the utilities      
utils: all $(UTILS_BIN_DIR) $(UTILS_BINS) test_sets_copy_utils

# TARGET: Builds the developers documentation    
doc:
		cd $(DEV_DOC_DIR); doxygen Doxyfile

# TARGET: Clean the build	
clean:
		rm -f -r $(BIN_DIR)
		
# TARGET: Builds a library distribution/release		
dist:
		rm -f -r $(PROJECT_ROOT)/../../files/exip-$(EXIP_VERSION)
		svn export $(PROJECT_ROOT) $(PROJECT_ROOT)/../../files/exip-$(EXIP_VERSION)
		cd $(PROJECT_ROOT)/../../files && tar cvf - exip-$(EXIP_VERSION) | gzip > exip-$(EXIP_VERSION).tar.gz
		cd $(PROJECT_ROOT)/../../files && zip -lvr exip-$(EXIP_VERSION).zip exip-$(EXIP_VERSION)
		
# Generating Prerequisites Automatically
# for more information see: http://www.gnu.org/software/make/manual/make.html#Automatic-Prerequisites
$(BIN_DIR)/%.d: %.c
	@set -e; rm -f $@; mkdir -p $(BIN_DIR); \
	$(COMPILE) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@D)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$		

include $(SOURCES_ALL:%.c=$(BIN_DIR)/%.d)

# A meta-rule for compiling any "C" source file.
$(BIN_DIR)/%.o: %.c
		$(COMPILE) -c $< -o $@

# Build the tests
$(TESTS_BIN_DIR)/test_%: $(BIN_DIR)/check_%.o
		$(COMPILE) $(LDFLAGS) $< -lcheck -lexip -o $@
		
# Build for the examples		
$(EXAMPLES_BIN_DIR)/exipe: $(EXIPE_OBJECTS)
		$(COMPILE) $(LDFLAGS) $^ -lexip -o $@
		
$(EXAMPLES_BIN_DIR)/exipd: $(EXIPD_OBJECTS)
		$(COMPILE) $(LDFLAGS) $^ -lexip -o $@	
		
# Build for the utils
$(UTILS_BIN_DIR)/exipg: $(EXIPG_OBJECTS)
		$(COMPILE) $(LDFLAGS) $^ -lexip -o $@
	
$(LIB_BIN_DIR)/libexip.a: $(LIB_OBJECTS)
		ar rcs $(LIB_BIN_DIR)/libexip.a $(LIB_OBJECTS)

$(LIB_BIN_DIR)/libexip.so: $(LIB_OBJECTS)
		$(CC) -shared -Wl,-soname,libexip.so.$(EXIP_VERSION) -o $(LIB_BIN_DIR)/libexip.so.$(EXIP_VERSION) $(LIB_OBJECTS)
		ln -sf $(LIB_BIN_DIR)/libexip.so.$(EXIP_VERSION) $(LIB_BIN_DIR)/libexip.so

$(BIN_DIR):
		mkdir -p $(BIN_DIR)
		
$(LIB_BIN_DIR):
		mkdir -p $(LIB_BIN_DIR)
		
$(EXAMPLES_BIN_DIR):
		mkdir -p $(EXAMPLES_BIN_DIR)
		
$(UTILS_BIN_DIR):
		mkdir -p $(UTILS_BIN_DIR)
		
$(TESTS_BIN_DIR):
		mkdir -p $(TESTS_BIN_DIR)

test_sets_copy_examples:
		cp $(PROJECT_ROOT)/examples/simpleEncoding/exipe-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipe-test-xsd.exi
		cp $(PROJECT_ROOT)/examples/simpleDecoding/exipd-test-xsd.exi $(EXAMPLES_BIN_DIR)/exipd-test-xsd.exi
		cp $(PROJECT_ROOT)/examples/simpleDecoding/exipd-test.exi $(EXAMPLES_BIN_DIR)/exipd-test.exi
		cp $(PROJECT_ROOT)/examples/simpleDecoding/exipd-test-schema.exi $(EXAMPLES_BIN_DIR)/exipd-test-schema.exi
		
test_sets_copy_utils:		
		cp $(PROJECT_ROOT)/utils/schemaHandling/EXIOptions-xsd.exi $(UTILS_BIN_DIR)/EXIOptions-xsd.exi	