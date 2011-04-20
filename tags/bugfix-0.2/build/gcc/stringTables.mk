# StreamIO module build

STRING_TABLES_OBJ = $(STRING_TABLES_BIN)/sTables.o

$(STRING_TABLES_BIN)/lib$(STRING_TABLES).a : $(STRING_TABLES_OBJ)
		ar rcs $(STRING_TABLES_BIN)/lib$(STRING_TABLES).a $(STRING_TABLES_OBJ)

$(STRING_TABLES_BIN)/sTables.o : $(STRING_TABLES_SRC)/include/sTables.h $(STRING_TABLES_SRC)/src/sTables.c
		$(CC) -c $(CFLAGS) $(STRING_TABLES_SRC)/src/sTables.c -o $(STRING_TABLES_BIN)/sTables.o