# Common module build

COMMON_OBJ = $(COMMON)/p_ASCII_stringManipulate.o $(COMMON)/procTypes.o $(COMMON)/contentHandler.o

$(COMMON)/lib$(COMMON).a : $(COMMON_OBJ)
		ar rcs $(COMMON)/lib$(COMMON).a $(COMMON_OBJ)

$(COMMON)/p_ASCII_stringManipulate.o : $(COMMON)/src/p_ASCII_stringManipulate.c $(COMMON)/include/stringManipulate.h $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h $(COMMON)/include/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/p_ASCII_stringManipulate.c -o $(COMMON)/p_ASCII_stringManipulate.o
	
$(COMMON)/procTypes.o : $(COMMON)/src/procTypes.c $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h $(COMMON)/include/exipConfig.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/procTypes.c -o $(COMMON)/procTypes.o
		
$(COMMON)/contentHandler.o : $(COMMON)/src/contentHandler.c $(COMMON)/include/procTypes.h $(COMMON)/include/errorHandle.h $(COMMON)/include/exipConfig.h $(COMMON)/include/contentHandler.h
		$(CC) -c $(CFLAGS) $(COMMON)/src/contentHandler.c -o $(COMMON)/contentHandler.o