# StreamIO module build

GRAMMAR_OBJ = $(BIN_DIR)/eventsEXI.o $(BIN_DIR)/grammarRules.o $(BIN_DIR)/grammars.o

$(BIN_DIR)/eventsEXI.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/src/eventsEXI.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/eventsEXI.c -o $(BIN_DIR)/eventsEXI.o

$(BIN_DIR)/grammarRules.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/include/grammarRules.h $(GRAMMAR_SRC)/src/grammarRules.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/grammarRules.c -o $(BIN_DIR)/grammarRules.o
		
$(BIN_DIR)/grammars.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/include/grammarRules.h $(GRAMMAR_SRC)/src/grammars.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/grammars.c -o $(BIN_DIR)/grammars.o