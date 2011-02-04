# Common module build

COMMON_OBJ = $(COMMON_BIN)/p_ASCII_stringManipulate.o $(COMMON_BIN)/procTypes.o \
			 $(COMMON_BIN)/contentHandler.o $(COMMON_BIN)/memManagement.o \
			 $(COMMON_BIN)/hashtable.o $(COMMON_BIN)/hashUtils.o $(COMMON_BIN)/dynamicArray.o

$(COMMON_BIN)/lib$(COMMON).a : $(COMMON_OBJ)
		ar rcs $(COMMON_BIN)/lib$(COMMON).a $(COMMON_OBJ)

$(COMMON_BIN)/p_ASCII_stringManipulate.o : $(COMMON_SRC)/src/p_ASCII_stringManipulate.c $(PUBLIC_INCLUDE_DIR)/stringManipulate.h $(PUBLIC_INCLUDE_DIR)/procTypes.h $(PUBLIC_INCLUDE_DIR)/errorHandle.h $(PUBLIC_INCLUDE_DIR)/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/p_ASCII_stringManipulate.c -o $(COMMON_BIN)/p_ASCII_stringManipulate.o
	
$(COMMON_BIN)/procTypes.o : $(COMMON_SRC)/src/procTypes.c $(PUBLIC_INCLUDE_DIR)/procTypes.h $(PUBLIC_INCLUDE_DIR)/errorHandle.h $(PUBLIC_INCLUDE_DIR)/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/procTypes.c -o $(COMMON_BIN)/procTypes.o
		
$(COMMON_BIN)/contentHandler.o : $(COMMON_SRC)/src/contentHandler.c $(PUBLIC_INCLUDE_DIR)/procTypes.h $(PUBLIC_INCLUDE_DIR)/errorHandle.h $(PUBLIC_INCLUDE_DIR)/exipConfig.h $(PUBLIC_INCLUDE_DIR)/contentHandler.h
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/contentHandler.c -o $(COMMON_BIN)/contentHandler.o
		
$(COMMON_BIN)/memManagement.o : $(PUBLIC_INCLUDE_DIR)/procTypes.h $(PUBLIC_INCLUDE_DIR)/errorHandle.h $(COMMON_SRC)/include/memManagement.h $(COMMON_SRC)/src/memManagement.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/memManagement.c -o $(COMMON_BIN)/memManagement.o
		
$(COMMON_BIN)/hashtable.o : $(COMMON_SRC)/include/hashtable.h $(COMMON_SRC)/include/hashtable_private.h $(COMMON_SRC)/src/hashtable.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/hashtable.c -o $(COMMON_BIN)/hashtable.o

$(COMMON_BIN)/hashUtils.o : $(COMMON_SRC)/include/hashUtils.h $(COMMON_SRC)/src/hashUtils.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/hashUtils.c -o $(COMMON_BIN)/hashUtils.o
		
$(COMMON_BIN)/dynamicArray.o: $(COMMON_SRC)/include/dynamicArray.h $(COMMON_SRC)/src/dynamicArray.c
		$(CC) -c $(CFLAGS) $(COMMON_SRC)/src/dynamicArray.c -o $(COMMON_BIN)/dynamicArray.o