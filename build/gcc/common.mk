# Common module build

COMMON_OBJ = $(COMMON_BIN)/p_ASCII_stringManipulate.o $(COMMON_BIN)/procTypes.o \
			 $(COMMON_BIN)/contentHandler.o $(COMMON_BIN)/memManagement.o \
			 $(COMMON_BIN)/hashtable.o $(COMMON_BIN)/hashUtils.o

$(COMMON_BIN)/lib$(COMMON).a : $(COMMON_OBJ)
		ar rcs $(COMMON_BIN)/lib$(COMMON).a $(COMMON_OBJ)

$(COMMON_BIN)/p_ASCII_stringManipulate.o : $(COMMON_SRC)/src/p_ASCII_stringManipulate.c $(COMMON_SRC)/include/stringManipulate.h $(COMMON_SRC)/include/procTypes.h $(COMMON_SRC)/include/errorHandle.h $(COMMON_SRC)/include/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/p_ASCII_stringManipulate.c -o $(COMMON_BIN)/p_ASCII_stringManipulate.o
	
$(COMMON_BIN)/procTypes.o : $(COMMON_SRC)/src/procTypes.c $(COMMON_SRC)/include/procTypes.h $(COMMON_SRC)/include/errorHandle.h $(COMMON_SRC)/include/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/procTypes.c -o $(COMMON_BIN)/procTypes.o
		
$(COMMON_BIN)/contentHandler.o : $(COMMON_SRC)/src/contentHandler.c $(COMMON_SRC)/include/procTypes.h $(COMMON_SRC)/include/errorHandle.h $(COMMON_SRC)/include/exipConfig.h $(COMMON_SRC)/include/contentHandler.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/contentHandler.c -o $(COMMON_BIN)/contentHandler.o
		
$(COMMON_BIN)/memManagement.o : $(COMMON_SRC)/include/procTypes.h $(COMMON_SRC)/include/errorHandle.h $(COMMON_SRC)/include/memManagement.h $(COMMON_SRC)/src/memManagement.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/memManagement.c -o $(COMMON_BIN)/memManagement.o
		
$(COMMON_BIN)/hashtable.o : $(COMMON_SRC)/include/hashtable.h $(COMMON_SRC)/include/hashtable_private.h $(COMMON_SRC)/src/hashtable.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/hashtable.c -o $(COMMON_BIN)/hashtable.o

$(COMMON_BIN)/hashUtils.o : $(COMMON_SRC)/include/hashUtils.h $(COMMON_SRC)/src/hashUtils.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/hashUtils.c -o $(COMMON_BIN)/hashUtils.o