# StreamIO module build

STREAM_IO_OBJ = $(STREAM_IO)/streamDecode.o $(STREAM_IO)/streamRead.o $(STREAM_IO)/streamWrite.o

$(STREAM_IO)/lib$(STREAM_IO).a : $(STREAM_IO_OBJ)
		ar rcs $(STREAM_IO)/lib$(STREAM_IO).a $(STREAM_IO_OBJ)

$(STREAM_IO)/streamDecode.o : $(STREAM_IO)/include/streamDecode.h $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamDecode.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamDecode.c -o $(STREAM_IO)/streamDecode.o

$(STREAM_IO)/streamRead.o : $(STREAM_IO)/include/streamRead.h $(STREAM_IO)/src/streamRead.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamRead.c -o $(STREAM_IO)/streamRead.o
		
$(STREAM_IO)/streamWrite.o : $(STREAM_IO)/include/streamWrite.h $(STREAM_IO)/src/streamWrite.c
		$(CC) -c $(CFLAGS) $(STREAM_IO)/src/streamWrite.c -o $(STREAM_IO)/streamWrite.o
		
$(STREAM_IO)/test: $(COMMON)/lib$(COMMON).a $(STREAM_IO)/lib$(STREAM_IO).a $(STREAM_IO)/tests/check_streamIO.c
		$(CC) $(CFLAGS) $(STREAM_IO)/tests/check_streamIO.c $(STREAM_IO)/lib$(STREAM_IO).a $(COMMON)/lib$(COMMON).a \
		$(CHECK_DIR)/libcheck.a -o $(STREAM_IO)/test