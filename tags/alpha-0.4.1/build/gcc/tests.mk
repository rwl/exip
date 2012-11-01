# Unit tests builds

$(TESTS_BIN_DIR)/test_$(STREAM_IO): $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_streamIO.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_streamIO.c $(OBJECT_ALL) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STREAM_IO)

$(TESTS_BIN_DIR)/test_$(GRAMMAR): $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_grammar.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_$(GRAMMAR)

$(TESTS_BIN_DIR)/test_$(STRING_TABLES): $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_stringTables.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_stringTables.c $(OBJECT_ALL) \
		-lcheck -o $(TESTS_BIN_DIR)/test_$(STRING_TABLES)

$(TESTS_BIN_DIR)/test_$(CONTENT_IO): $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_contentIO.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_contentIO.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_$(CONTENT_IO)
		
$(TESTS_BIN_DIR)/test_exip: $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_exip.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_exip.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_exip

$(TESTS_BIN_DIR)/test_builtin_grammar: $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_builtin_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_builtin_grammar.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_builtin_grammar

$(TESTS_BIN_DIR)/test_strict_grammar: $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_strict_grammar.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_strict_grammar.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_strict_grammar

$(TESTS_BIN_DIR)/test_emptyType: $(OBJECT_ALL) $(TESTS_SRC_DIR)/check_emptyType.c
		$(COMPILE) $(LDFLAGS) $(TESTS_SRC_DIR)/check_emptyType.c $(OBJECT_ALL) -lcheck -o $(TESTS_BIN_DIR)/test_emptyType