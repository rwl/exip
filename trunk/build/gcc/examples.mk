$(EXAMPLES_BIN_DIR)/$(EXAMPLE_1): $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(BIN_DIR)/libexip.a
		$(CC) $(CFLAGS) $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(BIN_DIR)/libexip.a -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1)
		
$(EXAMPLES_BIN_DIR)/$(EXAMPLE_2): $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(BIN_DIR)/libexip.a
		$(CC) $(CFLAGS) $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(BIN_DIR)/libexip.a -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2)