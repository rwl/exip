$(EXAMPLES_BIN_DIR)/$(EXAMPLE_1): $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c
		$(COMPILE) $(LDFLAGS) $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c -lexip -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_1)
		
$(EXAMPLES_BIN_DIR)/$(EXAMPLE_2): $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c
		$(COMPILE) $(LDFLAGS) $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c -lexip -o $(EXAMPLES_BIN_DIR)/$(EXAMPLE_2)