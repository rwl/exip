# ContentIO module build

CONTENT_IO_OBJ = $(BIN_DIR)/headerDecode.o $(BIN_DIR)/headerEncode.o \
				 $(BIN_DIR)/bodyDecode.o $(BIN_DIR)/EXIParser.o $(BIN_DIR)/EXISerializer.o \
				 $(BIN_DIR)/bodyEncode.o $(BIN_DIR)/staticEXIOptions.o \
				 $(BIN_DIR)/initSchemaInstance.o

$(BIN_DIR)/headerEncode.o : $(CONTENT_IO_SRC)/include/headerEncode.h $(CONTENT_IO_SRC)/src/headerEncode.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/headerEncode.c -o $(BIN_DIR)/headerEncode.o
		
$(BIN_DIR)/headerDecode.o : $(CONTENT_IO_SRC)/include/headerDecode.h $(CONTENT_IO_SRC)/src/headerDecode.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/headerDecode.c -o $(BIN_DIR)/headerDecode.o
		
$(BIN_DIR)/bodyDecode.o : $(CONTENT_IO_SRC)/include/bodyDecode.h $(CONTENT_IO_SRC)/src/bodyDecode.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/bodyDecode.c -o $(BIN_DIR)/bodyDecode.o
		
$(BIN_DIR)/EXIParser.o : $(PUBLIC_INCLUDE_DIR)/EXIParser.h $(CONTENT_IO_SRC)/src/EXIParser.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/EXIParser.c -o $(BIN_DIR)/EXIParser.o
		
$(BIN_DIR)/EXISerializer.o : $(PUBLIC_INCLUDE_DIR)/EXISerializer.h $(CONTENT_IO_SRC)/include/bodyEncode.h $(CONTENT_IO_SRC)/src/EXISerializer.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/EXISerializer.c -o $(BIN_DIR)/EXISerializer.o

$(BIN_DIR)/bodyEncode.o : $(CONTENT_IO_SRC)/include/bodyEncode.h $(CONTENT_IO_SRC)/src/bodyEncode.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/bodyEncode.c -o $(BIN_DIR)/bodyEncode.o
		
$(BIN_DIR)/staticEXIOptions.o : $(CONTENT_IO_SRC)/src/staticEXIOptions.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/staticEXIOptions.c -o $(BIN_DIR)/staticEXIOptions.o
		
$(BIN_DIR)/initSchemaInstance.o : $(CONTENT_IO_SRC)/src/initSchemaInstance.c
		$(COMPILE) -c $(CONTENT_IO_SRC)/src/initSchemaInstance.c -o $(BIN_DIR)/initSchemaInstance.o