# GrammarGen module build

GRAMMAR_GEN_OBJ = $(BIN_DIR)/genUtils.o $(BIN_DIR)/grammarGenerator.o $(BIN_DIR)/protoGrammars.o \
				$(BIN_DIR)/treeTableBuild.o $(BIN_DIR)/treeTableManipulate.o $(BIN_DIR)/treeTableToGrammars.o

$(BIN_DIR)/genUtils.o : $(GRAMMAR_GEN_SRC)/include/genUtils.h $(GRAMMAR_GEN_SRC)/src/genUtils.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/genUtils.c -o $(BIN_DIR)/genUtils.o

$(BIN_DIR)/grammarGenerator.o : $(PUBLIC_INCLUDE_DIR)/grammarGenerator.h $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c -o $(BIN_DIR)/grammarGenerator.o

$(BIN_DIR)/protoGrammars.o : $(GRAMMAR_GEN_SRC)/include/protoGrammars.h $(GRAMMAR_GEN_SRC)/src/protoGrammars.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/protoGrammars.c -o $(BIN_DIR)/protoGrammars.o
		
$(BIN_DIR)/treeTableBuild.o : $(GRAMMAR_GEN_SRC)/include/treeTableSchema.h $(GRAMMAR_GEN_SRC)/src/treeTableBuild.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/treeTableBuild.c -o $(BIN_DIR)/treeTableBuild.o
		
$(BIN_DIR)/treeTableManipulate.o : $(GRAMMAR_GEN_SRC)/include/treeTableSchema.h $(GRAMMAR_GEN_SRC)/src/treeTableManipulate.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/treeTableManipulate.c -o $(BIN_DIR)/treeTableManipulate.o
		
$(BIN_DIR)/treeTableToGrammars.o : $(GRAMMAR_GEN_SRC)/include/treeTableSchema.h $(GRAMMAR_GEN_SRC)/src/treeTableToGrammars.c
		$(COMPILE) -c $(GRAMMAR_GEN_SRC)/src/treeTableToGrammars.c -o $(BIN_DIR)/treeTableToGrammars.o