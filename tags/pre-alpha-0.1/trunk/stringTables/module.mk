# StreamIO module build

STRING_TABLES_OBJ = $(STRING_TABLES)/sTables.o

$(STRING_TABLES)/lib$(STRING_TABLES).a : $(STRING_TABLES_OBJ)
		ar rcs $(STRING_TABLES)/lib$(STRING_TABLES).a $(STRING_TABLES_OBJ)

$(STRING_TABLES)/sTables.o : $(STRING_TABLES)/include/sTables.h $(STRING_TABLES)/src/sTables.c
		$(CC) -c $(CFLAGS) $(STRING_TABLES)/src/sTables.c -o $(STRING_TABLES)/sTables.o
		
$(STRING_TABLES)/test: $(COMMON)/lib$(COMMON).a $(STRING_TABLES)/lib$(STRING_TABLES).a $(STRING_TABLES)/tests/check_stringTables.c
		$(CC) $(CFLAGS) $(STRING_TABLES)/tests/check_stringTables.c $(COMMON)/lib$(COMMON).a $(STRING_TABLES)/lib$(STRING_TABLES).a \
		$(CHECK_DIR)/libcheck.a -o $(STRING_TABLES)/test