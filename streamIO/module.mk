# StreamIO module build

STREAM_IO_OBJ = $(STREAM_IO)/streamDecode.o $(STREAM_IO)/streamRead.o \
				$(STREAM_IO)/streamWrite.o $(STREAM_IO)/ioUtil.o $(STREAM_IO)/streamEncode.o

$(STREAM_IO)/lib$(STREAM_IO).a : $(STREAM_IO_OBJ)
		ar rcs $(STREAM_IO)/lib$(STREAM_IO).a $(STREAM_IO_OBJ)

$(STREAM_IO)/streamDecode.o : $(STREAM_IO)/include/streamDecode.h $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamDecode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamDecode.c -o $(STREAM_IO)/streamDecode.o

$(STREAM_IO)/streamRead.o : $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamRead.c $(STREAM_IO)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamRead.c -o $(STREAM_IO)/streamRead.o
		
$(STREAM_IO)/streamWrite.o : $(STREAM_IO)/include/streamWrite.h $(STREAM_IO)/src/streamWrite.c $(STREAM_IO)/include/ioUtil.h
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamWrite.c -o $(STREAM_IO)/streamWrite.o
		
$(STREAM_IO)/ioUtil.o : $(STREAM_IO)/include/ioUtil.h $(STREAM_IO)/src/ioUtil.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/ioUtil.c -o $(STREAM_IO)/ioUtil.o
		
$(STREAM_IO)/streamEncode.o : $(STREAM_IO)/include/streamEncode.h $(STREAM_IO)/include/streamWrite.h $(STREAM_IO)/src/streamEncode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamEncode.c -o $(STREAM_IO)/streamEncode.o

$(STREAM_IO)/test: $(COMMON)/lib$(COMMON).a $(STREAM_IO)/lib$(STREAM_IO).a $(STREAM_IO)/tests/check_streamIO.c
		$(CC) $(CFLAGS) $(STREAM_IO)/tests/check_streamIO.c $(STREAM_IO)/lib$(STREAM_IO).a $(COMMON)/lib$(COMMON).a \
		$(CHECK_DIR)/libcheck.a -o $(STREAM_IO)/test