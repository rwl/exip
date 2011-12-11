# GrammarGen module build

GRAMMAR_GEN_OBJ = $(BIN_DIR)/genUtils.o $(BIN_DIR)/grammarGenerator.o $(BIN_DIR)/protoGrammars.o

$(BIN_DIR)/genUtils.o : $(GRAMMAR_GEN_SRC)/include/genUtils.h $(GRAMMAR_GEN_SRC)/src/genUtils.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/genUtils.c -o $(BIN_DIR)/genUtils.o

$(BIN_DIR)/grammarGenerator.o : $(PUBLIC_INCLUDE_DIR)/grammarGenerator.h $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c -o $(BIN_DIR)/grammarGenerator.o

$(BIN_DIR)/protoGrammars.o : $(GRAMMAR_GEN_SRC)/include/protoGrammars.h $(GRAMMAR_GEN_SRC)/src/protoGrammars.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/protoGrammars.c -o $(BIN_DIR)/protoGrammars.o