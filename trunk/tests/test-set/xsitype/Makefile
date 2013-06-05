
ROOT=$(ROOTACT)
#ROOT=$(HOME)

EXIP=$(ROOTACT)/exip
EXIPROCESSOR=$(ROOTACT)/ExiProcessor

CFLAGS += -I$(EXIP)/src/grammar/include -I$(EXIP)/include -I$(EXIP)/build/gcc/pc -I$(EXIP)/src/stringTables/include
LIB = $(EXIP)/bin/lib/libexip.so -lmxml -lpthread
LDFLAGS += -L/usr/local/lib -s

all	: schemaConvert.x Product.exs xsitypeProduct.x xsitypeM2M.x

xsitypeProduct.x : xsitypeProduct.o
	$(CC) -o xsitypeProduct.x xsitypeProduct.o $(LDFLAGS) $(LIB)

xsitypeM2M.x : xsitypeM2M.o
	$(CC) -o xsitypeM2M.x xsitypeM2M.o $(LDFLAGS) $(LIB)

schemaConvert.x : schemaConvert.o
	$(CC) -o schemaConvert.x schemaConvert.o

lint :
	xmllint --schema Product.xsd product.xml

test :
	./xsitypeProduct.x
	./xsitypeProduct.x -s

clean	:
	@(rm *.a *.so *.o *.x 2>/dev/null || exit 0)

%.o : %.c
	$(CC) $(CFLAGS) $(PROF) -c $<

%.exs : %.xsd
	java -jar ${EXIPROCESSOR}/ExiProcessor.jar -header_options -preserve_prefixes -xml_in $< -exi_out $@
	@./schemaConvert.x < $@ > __temp
	@mv __temp $@
