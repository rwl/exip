# StreamIO module build

GRAMMAR_OBJ = $(GRAMMAR_BIN)/eventsEXI.o $(GRAMMAR_BIN)/grammarRules.o $(GRAMMAR_BIN)/grammars.o

$(GRAMMAR_BIN)/eventsEXI.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/src/eventsEXI.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/eventsEXI.c -o $(GRAMMAR_BIN)/eventsEXI.o

$(GRAMMAR_BIN)/grammarRules.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/include/grammarRules.h $(GRAMMAR_SRC)/src/grammarRules.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/grammarRules.c -o $(GRAMMAR_BIN)/grammarRules.o
		
$(GRAMMAR_BIN)/grammars.o : $(GRAMMAR_SRC)/include/eventsEXI.h $(GRAMMAR_SRC)/include/grammarRules.h $(GRAMMAR_SRC)/src/grammars.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_SRC)/src/grammars.c -o $(GRAMMAR_BIN)/grammars.o