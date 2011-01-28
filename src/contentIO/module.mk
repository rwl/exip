# ContentIO module build

CONTENT_IO_OBJ = $(CONTENT_IO)/headerDecode.o $(CONTENT_IO)/headerEncode.o \
				 $(CONTENT_IO)/bodyDecode.o $(CONTENT_IO)/EXIParser.o $(CONTENT_IO)/EXISerializer.o

$(CONTENT_IO)/lib$(CONTENT_IO).a: $(CONTENT_IO_OBJ)
		ar rcs $(CONTENT_IO)/lib$(CONTENT_IO).a $(CONTENT_IO_OBJ)

$(CONTENT_IO)/headerEncode.o : $(CONTENT_IO)/include/headerEncode.h $(CONTENT_IO)/src/headerEncode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO)/src/headerEncode.c -o $(CONTENT_IO)/headerEncode.o
		
$(CONTENT_IO)/headerDecode.o : $(CONTENT_IO)/include/headerDecode.h $(CONTENT_IO)/src/headerDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO)/src/headerDecode.c -o $(CONTENT_IO)/headerDecode.o
		
$(CONTENT_IO)/bodyDecode.o : $(CONTENT_IO)/include/bodyDecode.h $(CONTENT_IO)/src/bodyDecode.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO)/src/bodyDecode.c -o $(CONTENT_IO)/bodyDecode.o
		
$(CONTENT_IO)/EXIParser.o : $(CONTENT_IO)/include/EXIParser.h $(CONTENT_IO)/src/EXIParser.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO)/src/EXIParser.c -o $(CONTENT_IO)/EXIParser.o
		
$(CONTENT_IO)/EXISerializer.o : $(CONTENT_IO)/include/EXISerializer.h $(CONTENT_IO)/src/EXISerializer.c
		$(CC) -c $(CFLAGS) $(CONTENT_IO)/src/EXISerializer.c -o $(CONTENT_IO)/EXISerializer.o
		
$(CONTENT_IO)/test: $(CONTENT_IO)/lib$(CONTENT_IO).a $(COMMON)/lib$(COMMON).a $(STREAM_IO)/lib$(STREAM_IO).a $(CONTENT_IO)/tests/check_contentIO.c
		$(CC) $(CFLAGS) $(CONTENT_IO)/tests/check_contentIO.c $(CONTENT_IO)/lib$(CONTENT_IO).a $(COMMON)/lib$(COMMON).a $(STREAM_IO)/lib$(STREAM_IO).a $(CHECK_DIR)/libcheck.a -o $(CONTENT_IO)/test