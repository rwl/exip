$(EXAMPLES_BIN_DIR)/exipd: $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(EXAMPLES_SRC_DIR)/simpleDecoding/exipd.c $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.h
		$(COMPILE) $(LDFLAGS) $(EXAMPLES_SRC_DIR)/simpleDecoding/decodeTestEXI.c $(EXAMPLES_SRC_DIR)/simpleDecoding/exipd.c -lexip -o $(EXAMPLES_BIN_DIR)/exipd
		
$(EXAMPLES_BIN_DIR)/exipe: $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(EXAMPLES_SRC_DIR)/simpleEncoding/exipe.c $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.h
		$(COMPILE) $(LDFLAGS) $(EXAMPLES_SRC_DIR)/simpleEncoding/encodeTestEXI.c $(EXAMPLES_SRC_DIR)/simpleEncoding/exipe.c -lexip -o $(EXAMPLES_BIN_DIR)/exipe