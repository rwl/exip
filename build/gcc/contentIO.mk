# ContentIO module build

CONTENT_IO_OBJ = $(BIN_DIR)/headerDecode.o $(BIN_DIR)/headerEncode.o \
				 $(BIN_DIR)/bodyDecode.o $(BIN_DIR)/EXIParser.o $(BIN_DIR)/EXISerializer.o \
				 $(BIN_DIR)/bodyEncode.o $(BIN_DIR)/grammarAugment.o $(BIN_DIR)/staticEXIOptions.o

$(BIN_DIR)/headerEncode.o : $(CONTENT_IO_SRC)/include/headerEncode.h $(CONTENT_IO_SRC)/src/headerEncode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/headerEncode.c -o $(BIN_DIR)/headerEncode.o
		
$(BIN_DIR)/headerDecode.o : $(CONTENT_IO_SRC)/include/headerDecode.h $(CONTENT_IO_SRC)/src/headerDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/headerDecode.c -o $(BIN_DIR)/headerDecode.o
		
$(BIN_DIR)/bodyDecode.o : $(CONTENT_IO_SRC)/include/bodyDecode.h $(CONTENT_IO_SRC)/src/bodyDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/bodyDecode.c -o $(BIN_DIR)/bodyDecode.o
		
$(BIN_DIR)/EXIParser.o : $(PUBLIC_INCLUDE_DIR)/EXIParser.h $(CONTENT_IO_SRC)/src/EXIParser.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/EXIParser.c -o $(BIN_DIR)/EXIParser.o
		
$(BIN_DIR)/EXISerializer.o : $(PUBLIC_INCLUDE_DIR)/EXISerializer.h $(CONTENT_IO_SRC)/include/bodyEncode.h $(CONTENT_IO_SRC)/src/EXISerializer.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/EXISerializer.c -o $(BIN_DIR)/EXISerializer.o

$(BIN_DIR)/bodyEncode.o : $(CONTENT_IO_SRC)/include/bodyEncode.h $(CONTENT_IO_SRC)/src/bodyEncode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/bodyEncode.c -o $(BIN_DIR)/bodyEncode.o
		
$(BIN_DIR)/grammarAugment.o : $(CONTENT_IO_SRC)/include/grammarAugment.h $(CONTENT_IO_SRC)/src/grammarAugment.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/grammarAugment.c -o $(BIN_DIR)/grammarAugment.o
		
$(BIN_DIR)/staticEXIOptions.o : $(CONTENT_IO_SRC)/src/staticEXIOptions.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO_SRC)/src/staticEXIOptions.c -o $(BIN_DIR)/staticEXIOptions.o