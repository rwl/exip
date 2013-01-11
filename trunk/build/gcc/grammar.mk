# StreamIO module build

GRAMMAR_OBJ = $(BIN_DIR)/grammars.o

$(BIN_DIR)/grammars.o : $(GRAMMAR_SRC)/include/grammars.h $(GRAMMAR_SRC)/src/grammars.c
		$(COMPILE) -c $(GRAMMAR_SRC)/src/grammars.c -o $(BIN_DIR)/grammars.o