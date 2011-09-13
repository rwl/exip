# StreamIO module build

STREAM_IO_OBJ = $(BIN_DIR)/streamDecode.o $(BIN_DIR)/streamRead.o \
				$(BIN_DIR)/streamWrite.o $(BIN_DIR)/ioUtil.o $(BIN_DIR)/streamEncode.o

$(BIN_DIR)/streamDecode.o : $(STREAM_IO_SRC)/include/streamDecode.h $(STREAM_IO_SRC)/include/streamRead.h $(STREAM_IO_SRC)/src/streamDecode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamDecode.c -o $(BIN_DIR)/streamDecode.o

$(BIN_DIR)/streamRead.o : $(STREAM_IO_SRC)/include/streamRead.h $(STREAM_IO_SRC)/src/streamRead.c $(STREAM_IO_SRC)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamRead.c -o $(BIN_DIR)/streamRead.o
		
$(BIN_DIR)/streamWrite.o : $(STREAM_IO_SRC)/include/streamWrite.h $(STREAM_IO_SRC)/src/streamWrite.c $(STREAM_IO_SRC)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamWrite.c -o $(BIN_DIR)/streamWrite.o
		
$(BIN_DIR)/ioUtil.o : $(STREAM_IO_SRC)/include/ioUtil.h $(STREAM_IO_SRC)/src/ioUtil.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/ioUtil.c -o $(BIN_DIR)/ioUtil.o
		
$(BIN_DIR)/streamEncode.o : $(STREAM_IO_SRC)/include/streamEncode.h $(STREAM_IO_SRC)/include/streamWrite.h $(STREAM_IO_SRC)/src/streamEncode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamEncode.c -o $(BIN_DIR)/streamEncode.o