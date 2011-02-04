# ContentIO module build

CONTENT_IO_OBJ = $(CONTENT_IO_BIN)/headerDecode.o $(CONTENT_IO_BIN)/headerEncode.o \
				 $(CONTENT_IO_BIN)/bodyDecode.o $(CONTENT_IO_BIN)/EXIParser.o $(CONTENT_IO_BIN)/EXISerializer.o

$(CONTENT_IO_BIN)/lib$(CONTENT_IO).a: $(CONTENT_IO_OBJ)
		ar rcs $(CONTENT_IO_BIN)/lib$(CONTENT_IO).a $(CONTENT_IO_OBJ)

$(CONTENT_IO_BIN)/headerEncode.o : $(CONTENT_IO_SRC)/include/headerEncode.h $(CONTENT_IO_SRC)/src/headerEncode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/headerEncode.c -o $(CONTENT_IO_BIN)/headerEncode.o
		
$(CONTENT_IO_BIN)/headerDecode.o : $(CONTENT_IO_SRC)/include/headerDecode.h $(CONTENT_IO_SRC)/src/headerDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/headerDecode.c -o $(CONTENT_IO_BIN)/headerDecode.o
		
$(CONTENT_IO_BIN)/bodyDecode.o : $(CONTENT_IO_SRC)/include/bodyDecode.h $(CONTENT_IO_SRC)/src/bodyDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/bodyDecode.c -o $(CONTENT_IO_BIN)/bodyDecode.o
		
$(CONTENT_IO_BIN)/EXIParser.o : $(PUBLIC_INCLUDE_DIR)/EXIParser.h $(CONTENT_IO_SRC)/src/EXIParser.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/EXIParser.c -o $(CONTENT_IO_BIN)/EXIParser.o
		
$(CONTENT_IO_BIN)/EXISerializer.o : $(PUBLIC_INCLUDE_DIR)/EXISerializer.h $(CONTENT_IO_SRC)/include/bodyEncode.h $(CONTENT_IO_SRC)/src/EXISerializer.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/EXISerializer.c -o $(CONTENT_IO_BIN)/EXISerializer.o