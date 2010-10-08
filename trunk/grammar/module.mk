# StreamIO module build

GRAMMAR_OBJ = $(GRAMMAR)/eventsEXI.o $(GRAMMAR)/grammarRules.o $(GRAMMAR)/grammars.o

$(GRAMMAR)/lib$(GRAMMAR).a : $(GRAMMAR_OBJ)
		ar rcs $(GRAMMAR)/lib$(GRAMMAR).a $(GRAMMAR_OBJ)

$(GRAMMAR)/eventsEXI.o : $(GRAMMAR)/include/eventsEXI.h $(GRAMMAR)/src/eventsEXI.c
		$(CC) -c $(CFLAGS) $(GRAMMAR)/src/eventsEXI.c -o $(GRAMMAR)/eventsEXI.o

$(GRAMMAR)/grammarRules.o : $(GRAMMAR)/include/eventsEXI.h $(GRAMMAR)/include/grammarRules.h $(GRAMMAR)/src/grammarRules.c
		$(CC) -c $(CFLAGS) $(GRAMMAR)/src/grammarRules.c -o $(GRAMMAR)/grammarRules.o
		
$(GRAMMAR)/grammars.o : $(GRAMMAR)/include/eventsEXI.h $(GRAMMAR)/include/grammarRules.h $(GRAMMAR)/src/grammars.c
		$(CC) -c $(CFLAGS) $(GRAMMAR)/src/grammars.c -o $(GRAMMAR)/grammars.o
		
$(GRAMMAR)/test: $(COMMON)/lib$(COMMON).a $(STREAM_IO)/lib$(STREAM_IO).a $(GRAMMAR)/lib$(GRAMMAR).a $(STRING_TABLES)/lib$(STRING_TABLES).a $(GRAMMAR)/tests/check_grammar.c
		$(CC) $(CFLAGS) $(GRAMMAR)/tests/check_grammar.c $(GRAMMAR)/lib$(GRAMMAR).a $(STREAM_IO)/lib$(STREAM_IO).a \
        $(COMMON)/lib$(COMMON).a $(CHECK_DIR)/libcheck.a $(STRING_TABLES)/lib$(STRING_TABLES).a -o $(GRAMMAR)/test