# StreamIO module build

STRING_TABLES_OBJ = $(BIN_DIR)/sTables.o

$(BIN_DIR)/sTables.o : $(STRING_TABLES_SRC)/include/sTables.h $(STRING_TABLES_SRC)/src/sTables.c
		$(CC) -c $(CFLAGS) $(STRING_TABLES_SRC)/src/sTables.c -o $(BIN_DIR)/sTables.o