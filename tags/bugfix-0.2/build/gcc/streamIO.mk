# StreamIO module build

STREAM_IO_OBJ = $(STREAM_IO_BIN)/streamDecode.o $(STREAM_IO_BIN)/streamRead.o \
				$(STREAM_IO_BIN)/streamWrite.o $(STREAM_IO_BIN)/ioUtil.o $(STREAM_IO_BIN)/streamEncode.o

$(STREAM_IO_BIN)/lib$(STREAM_IO).a : $(STREAM_IO_OBJ)
		ar rcs $(STREAM_IO_BIN)/lib$(STREAM_IO).a $(STREAM_IO_OBJ)

$(STREAM_IO_BIN)/streamDecode.o : $(STREAM_IO_SRC)/include/streamDecode.h $(STREAM_IO_SRC)/include/streamRead.h $(STREAM_IO_SRC)/src/streamDecode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamDecode.c -o $(STREAM_IO_BIN)/streamDecode.o

$(STREAM_IO_BIN)/streamRead.o : $(STREAM_IO_SRC)/include/streamRead.h $(STREAM_IO_SRC)/src/streamRead.c $(STREAM_IO_SRC)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamRead.c -o $(STREAM_IO_BIN)/streamRead.o
		
$(STREAM_IO_BIN)/streamWrite.o : $(STREAM_IO_SRC)/include/streamWrite.h $(STREAM_IO_SRC)/src/streamWrite.c $(STREAM_IO_SRC)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamWrite.c -o $(STREAM_IO_BIN)/streamWrite.o
		
$(STREAM_IO_BIN)/ioUtil.o : $(STREAM_IO_SRC)/include/ioUtil.h $(STREAM_IO_SRC)/src/ioUtil.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/ioUtil.c -o $(STREAM_IO_BIN)/ioUtil.o
		
$(STREAM_IO_BIN)/streamEncode.o : $(STREAM_IO_SRC)/include/streamEncode.h $(STREAM_IO_SRC)/include/streamWrite.h $(STREAM_IO_SRC)/src/streamEncode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO_SRC)/src/streamEncode.c -o $(STREAM_IO_BIN)/streamEncode.o