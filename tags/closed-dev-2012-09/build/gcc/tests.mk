# Unit tests builds

$(TESTS_BIN_DIR)/test_$(STREAM_IO): $(COMMON_OBJ) $(STREAM_IO_OBJ) $(TESTS_SRC_DIR)/check_streamIO.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_streamIO.c $(STREAM_IO_OBJ) $(COMMON_OBJ) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STREAM_IO)

$(TESTS_BIN_DIR)/test_$(GRAMMAR): $(COMMON_OBJ) $(STREAM_IO_OBJ) $(GRAMMAR_OBJ) $(STRING_TABLES_OBJ) $(CONTENT_IO_OBJ) $(TESTS_SRC_DIR)/check_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_grammar.c $(GRAMMAR_OBJ) $(STREAM_IO_OBJ) \
		$(CONTENT_IO_OBJ) $(COMMON_OBJ) $(STRING_TABLES_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_$(GRAMMAR)

$(TESTS_BIN_DIR)/test_$(STRING_TABLES): $(COMMON_OBJ) $(STRING_TABLES_OBJ) $(TESTS_SRC_DIR)/check_stringTables.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_stringTables.c $(COMMON_OBJ) $(STRING_TABLES_OBJ) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STRING_TABLES)

$(TESTS_BIN_DIR)/test_$(CONTENT_IO): $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(TESTS_SRC_DIR)/check_contentIO.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_contentIO.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_$(CONTENT_IO)
		
$(TESTS_BIN_DIR)/test_exip: $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(TESTS_SRC_DIR)/check_exip.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_exip.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_exip

$(TESTS_BIN_DIR)/test_builtin_grammar: $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(TESTS_SRC_DIR)/check_builtin_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_builtin_grammar.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_builtin_grammar

$(TESTS_BIN_DIR)/test_strict_grammar: $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(GRAMMAR_GEN_OBJ) $(TESTS_SRC_DIR)/check_strict_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_strict_grammar.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(GRAMMAR_GEN_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_strict_grammar

$(TESTS_BIN_DIR)/test_emptyType: $(COMMON_OBJ) $(STREAM_IO_OBJ) $(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(GRAMMAR_GEN_OBJ) $(TESTS_SRC_DIR)/check_emptyType.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_emptyType.c $(COMMON_OBJ) $(STREAM_IO_OBJ) \
		$(STRING_TABLES_OBJ) $(GRAMMAR_OBJ) $(CONTENT_IO_OBJ) $(GRAMMAR_GEN_OBJ) -lcheck -o $(TESTS_BIN_DIR)/test_emptyType