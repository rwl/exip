# GrammarGen module build

GRAMMAR_GEN_OBJ = $(GRAMMAR_GEN_BIN)/genUtils.o $(GRAMMAR_GEN_BIN)/grammarAugment.o \
				  $(GRAMMAR_GEN_BIN)/grammarGenerator.o $(GRAMMAR_GEN_BIN)/normGrammar.o

$(GRAMMAR_GEN_BIN)/lib$(GRAMMAR_GEN).a : $(GRAMMAR_GEN_OBJ)
		ar rcs $(GRAMMAR_GEN_BIN)/lib$(GRAMMAR_GEN).a $(GRAMMAR_GEN_OBJ)

$(GRAMMAR_GEN_BIN)/genUtils.o : $(GRAMMAR_GEN_SRC)/include/genUtils.h $(GRAMMAR_GEN_SRC)/src/genUtils.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/genUtils.c -o $(GRAMMAR_GEN_BIN)/genUtils.o

$(GRAMMAR_GEN_BIN)/grammarAugment.o : $(GRAMMAR_GEN_SRC)/include/grammarAugment.h $(GRAMMAR_GEN_SRC)/src/grammarAugment.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/grammarAugment.c -o $(GRAMMAR_GEN_BIN)/grammarAugment.o
		
$(GRAMMAR_GEN_BIN)/grammarGenerator.o : $(PUBLIC_INCLUDE_DIR)/grammarGenerator.h $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c $(PUBLIC_INCLUDE_DIR)/schema.h
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/grammarGenerator.c -o $(GRAMMAR_GEN_BIN)/grammarGenerator.o

$(GRAMMAR_GEN_BIN)/normGrammar.o : $(GRAMMAR_GEN_SRC)/include/normGrammar.h $(GRAMMAR_GEN_SRC)/src/normGrammar.c
		$(CC) -c $(CFLAGS) $(GRAMMAR_GEN_SRC)/src/normGrammar.c -o $(GRAMMAR_GEN_BIN)/normGrammar.o