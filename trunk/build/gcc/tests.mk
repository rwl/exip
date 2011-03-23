# Unit tests builds

$(TESTS_BIN_DIR)/test_$(STREAM_IO): common streamIO $(TESTS_SRC_DIR)/check_streamIO.c
		$(CC) $(CFLAGS) $(TESTS_SRC_DIR)/check_streamIO.c $(STREAM_IO_BIN)/lib$(STREAM_IO).a $(COMMON_BIN)/lib$(COMMON).a \
		$(CHECK_DIR)/libcheck.a -o $(TESTS_BIN_DIR)/test_$(STREAM_IO)

$(TESTS_BIN_DIR)/test_$(GRAMMAR): common streamIO grammar stringTables contentIO $(TESTS_SRC_DIR)/check_grammar.c
		$(CC) $(CFLAGS) $(TESTS_SRC_DIR)/check_grammar.c $(GRAMMAR_BIN)/lib$(GRAMMAR).a $(STREAM_IO_BIN)/lib$(STREAM_IO).a $(CONTENT_IO_BIN)/lib$(CONTENT_IO).a \
        $(COMMON_BIN)/lib$(COMMON).a $(CHECK_DIR)/libcheck.a $(STRING_TABLES_BIN)/lib$(STRING_TABLES).a -o $(TESTS_BIN_DIR)/test_$(GRAMMAR)

$(TESTS_BIN_DIR)/test_$(STRING_TABLES): common stringTables $(TESTS_SRC_DIR)/check_stringTables.c
		$(CC) $(CFLAGS) $(TESTS_SRC_DIR)/check_stringTables.c $(COMMON_BIN)/lib$(COMMON).a $(STRING_TABLES_BIN)/lib$(STRING_TABLES).a \
		$(CHECK_DIR)/libcheck.a -o $(TESTS_BIN_DIR)/test_$(STRING_TABLES)

$(TESTS_BIN_DIR)/test_$(CONTENT_IO): contentIO common streamIO $(TESTS_SRC_DIR)/check_contentIO.c
		$(CC) $(CFLAGS) $(TESTS_SRC_DIR)/check_contentIO.c $(CONTENT_IO_BIN)/lib$(CONTENT_IO).a $(COMMON_BIN)/lib$(COMMON).a $(STREAM_IO_BIN)/lib$(STREAM_IO).a $(CHECK_DIR)/libcheck.a -o $(TESTS_BIN_DIR)/test_$(CONTENT_IO)