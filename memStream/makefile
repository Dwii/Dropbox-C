CC=gcc -fPIC
AR=ar
LD_LIBRARY_PATH=/usr/local/include


OBJ_PATH=obj
SRC_PATH=src
OUT_PATH=out
INCLUDE_PATH=include

LIBRARY_INSTALL_PATH=/usr/local/lib
INCLUDE_INSTALL_PATH=/usr/local/include

FLAGS=-g -Wall -std=gnu99 -I $(LD_LIBRARY_PATH)

OBJ=$(OBJ_PATH)/memStream.o
OUT=$(OUT_PATH)/libmemstream.a

all: EXPORT_VAR $(OBJ_PATH) $(OUT_PATH) $(LIBRARY_INSTALL_PATH) $(INCLUDE_INSTALL_PATH) $(OUT)

EXPORT_VAR:
	-ldconfig $(LIBRARY_INSTALL_PATH)

strip: $(OUT)
	-strip $^


$(OUT): $(OBJ)
	$(AR) rcs $@ $^ 

$(OBJ): $(SRC_PATH)/memStream.c $(INCLUDE_PATH)/memStream.h
	$(CC) $(FLAGS) -o $@ -c $< -I $(INCLUDE_PATH)

$(OBJ_PATH) $(OUT_PATH) $(LIBRARY_INSTALL_PATH) $(INCLUDE_INSTALL_PATH):
	-mkdir -p $@

.PHONY install: all
	install -m 0755 $(OUT) $(LIBRARY_INSTALL_PATH)
	install -m 0644 $(INCLUDE_PATH)/memStream.h $(INCLUDE_INSTALL_PATH)

uninstall: 
	rm -rf $(LIBRARY_INSTALL_PATH)/libmemstream.a $(INCLUDE_INSTALL_PATH)/memStream.h

clean:
	rm -rf $(OBJ) $(OUT)

rebuild: clean all
