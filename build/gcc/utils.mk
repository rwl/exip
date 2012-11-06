# Utils build

# Build exipg:
EXIPG_INCL_DIR = -I$(UTILS_SRC_DIR)/schemaHandling/include

EXIPG_OBJ = $(UTILS_BIN_DIR)/dynOutputUtils.o $(UTILS_BIN_DIR)/textOutputUtils.o \
			$(UTILS_BIN_DIR)/exipOutputUtils.o $(UTILS_BIN_DIR)/staticOutputUtils.o $(UTILS_BIN_DIR)/createGrammars.o

$(UTILS_BIN_DIR)/exipg: $(UTILS_SRC_DIR)/schemaHandling/exipg.c $(EXIPG_OBJ) $(UTILS_SRC_DIR)/schemaHandling/include/*.*
		$(COMPILE) $(LDFLAGS) $(EXIPG_INCL_DIR) $(UTILS_SRC_DIR)/schemaHandling/exipg.c $(EXIPG_OBJ) -lexip -o $(UTILS_BIN_DIR)/exipg
		
$(UTILS_BIN_DIR)/dynOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/output/dynOutputUtils.c
		$(COMPILE) $(EXIPG_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/output/dynOutputUtils.c -o $(UTILS_BIN_DIR)/dynOutputUtils.o

$(UTILS_BIN_DIR)/textOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/output/textOutputUtils.c
		$(COMPILE) $(EXIPG_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/output/textOutputUtils.c -o $(UTILS_BIN_DIR)/textOutputUtils.o

$(UTILS_BIN_DIR)/exipOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/output/exipOutputUtils.c
		$(COMPILE) $(EXIPG_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/output/exipOutputUtils.c -o $(UTILS_BIN_DIR)/exipOutputUtils.o

$(UTILS_BIN_DIR)/staticOutputUtils.o: $(UTILS_SRC_DIR)/schemaHandling/output/staticOutputUtils.c
		$(COMPILE) $(EXIPG_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/output/staticOutputUtils.c -o $(UTILS_BIN_DIR)/staticOutputUtils.o
		
$(UTILS_BIN_DIR)/createGrammars.o: $(UTILS_SRC_DIR)/schemaHandling/createGrammars.c
		$(COMPILE) $(EXIPG_INCL_DIR) -c $(UTILS_SRC_DIR)/schemaHandling/createGrammars.c -o $(UTILS_BIN_DIR)/createGrammars.o		