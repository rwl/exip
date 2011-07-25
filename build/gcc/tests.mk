# Unit tests builds

$(TESTS_BIN_DIR)/test_$(STREAM_IO): common streamIO $(TESTS_SRC_DIR)/check_streamIO.c
		$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_SRC_DIR)/check_streamIO.c $(STREAM_IO_OBJ) $(COMMON_OBJ) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STREAM_IO)

$(TESTS_BIN_DIR)/test_$(GRAMMAR): common streamIO grammar stringTables contentIO $(TESTS_SRC_DIR)/check_grammar.c
		$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_SRC_DIR)/check_grammar.c $(GRAMMAR_OBJ) $(STREAM_IO_OBJ) \
		$(CONTENT_IO_OBJ) $(COMMON_OBJ) $(STRING_TABLES_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_$(GRAMMAR)

$(TESTS_BIN_DIR)/test_$(STRING_TABLES): common stringTables $(TESTS_SRC_DIR)/check_stringTables.c
		$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_SRC_DIR)/check_stringTables.c $(COMMON_OBJ) $(STRING_TABLES_OBJ) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STRING_TABLES)

$(TESTS_BIN_DIR)/test_$(CONTENT_IO): common streamIO stringTables grammar contentIO $(TESTS_SRC_DIR)/check_contentIO.c
		$(CC) $(CFLAGS) $(LDFLAGS) $(TESTS_SRC_DIR)/check_contentIO.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_$(CONTENT_IO)