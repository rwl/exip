# Utils build

UTILS_INCL_DIR = -I$(UTILS_SRC_DIR)/schemaHandling/utils/include

UTILS_1_OBJ = $(UTILS_BIN_DIR)/dynOutputUtils.o $(UTILS_BIN_DIR)/textOutputUtils.o \
			$(UTILS_BIN_DIR)/exipOutputUtils.o $(UTILS_BIN_DIR)/staticOutputUtils.o

$(UTILS_BIN_DIR)/$(UTILS_1): $(UTILS_SRC_DIR)/schemaHandling/createGrammars.c $(UTILS_1_OBJ)
		$(COMPILE) $(LDFLAGS) $(UTILS_INCL_DIR) $(UTILS_SRC_DIR)/schemaHandling/createGrammars.c $(UTILS_1_OBJ) -lexip -o $(UTILS_BIN_DIR)/$(UTILS_1)
		
$(UTILS_BIN_DIR)/dynOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/utils/src/dynOutputUtils.c
		$(COMPILE) $(UTILS_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/utils/src/dynOutputUtils.c -o $(UTILS_BIN_DIR)/dynOutputUtils.o

$(UTILS_BIN_DIR)/textOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/utils/src/textOutputUtils.c
		$(COMPILE) $(UTILS_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/utils/src/textOutputUtils.c -o $(UTILS_BIN_DIR)/textOutputUtils.o

$(UTILS_BIN_DIR)/exipOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/utils/src/exipOutputUtils.c
		$(COMPILE) $(UTILS_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/utils/src/exipOutputUtils.c -o $(UTILS_BIN_DIR)/exipOutputUtils.o

$(UTILS_BIN_DIR)/staticOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/utils/src/staticOutputUtils.c
		$(COMPILE) $(UTILS_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/utils/src/staticOutputUtils.c -o $(UTILS_BIN_DIR)/staticOutputUtils.o