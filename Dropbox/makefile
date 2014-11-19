CC=gcc -fPIC
AR=ar
LD_LIBRARY_PATH=/usr/local/include


OBJ_PATH=obj
SRC_PATH=src
OUT_PATH=out
INCLUDE_PATH=include
EXAMPLE_PATH=example

LIBRARY_INSTALL_PATH=/usr/local/lib
INCLUDE_INSTALL_PATH=/usr/local/include

FLAGS=-g -Wall -std=gnu99 -I $(LD_LIBRARY_PATH)

OBJ=$(addprefix $(OBJ_PATH)/,dropbox.o dropboxJson.o dropboxOAuth.o dropboxUtils.o dropboxUtils.o)
OUT=$(OUT_PATH)/libdropbox.so

DROPBOX_H       = $(addprefix $(INCLUDE_PATH)/, dropbox.h dropboxOAuth.h dropboxJson.h dropboxUtils.h)
DROPBOX_JSON_H  = $(addprefix $(INCLUDE_PATH)/, dropboxJson.h)
DROPBOX_OAUTH_H = $(addprefix $(INCLUDE_PATH)/, dropboxOAuth.h dropboxUtils.h)
DROPBOX_UTILS_H = $(addprefix $(INCLUDE_PATH)/, dropboxUtils.h)
EXAMPLE=$(EXAMPLE_PATH)/example

all: EXPORT_VAR $(OBJ_PATH) $(OUT_PATH) $(LIBRARY_INSTALL_PATH) $(INCLUDE_INSTALL_PATH) $(OUT)

example: $(EXAMPLE_PATH)/example

EXPORT_VAR: 
	-ldconfig $(LIBRARY_INSTALL_PATH)

strip: $(OUT)
	-strip $^

$(EXAMPLE): $(EXAMPLE_PATH)/example.c
	$(CC) $(FLAGS) $< -o $@ -Bstatic -lmemstream -Bdynamic -ldropbox -L$(LIBRARY_INSTALL_PATH)

$(OUT): $(OBJ)
	$(CC) $(FLAGS) -shared $^ -o $@ -I $(INCLUDE_PATH) -Bstatic -lmemstream -Bdynamic -lcurl -loauth -ljansson -L$(LIBRARY_INSTALL_PATH)

$(OBJ_PATH)/dropbox.o : $(SRC_PATH)/dropbox.c $(DROPBOX_H)
	$(CC) $(FLAGS) -o $@ -c $< -I $(INCLUDE_PATH)

$(OBJ_PATH)/dropboxJson.o : $(SRC_PATH)/dropboxJson.c $(DROPBOX_JSON_H)
	$(CC) $(FLAGS) -o $@ -c $< -I $(INCLUDE_PATH)

$(OBJ_PATH)/dropboxOAuth.o : $(SRC_PATH)/dropboxOAuth.c $(DROPBOX_OAUTH_H)
	$(CC) $(FLAGS) -o $@ -c $< -I $(INCLUDE_PATH)

$(OBJ_PATH)/dropboxUtils.o : $(SRC_PATH)/dropboxUtils.c $(DROPBOX_UTILS_H)
	$(CC) $(FLAGS) -o $@ -c $< -I $(INCLUDE_PATH)

$(OBJ_PATH) $(OUT_PATH) $(LIBRARY_INSTALL_PATH) $(INCLUDE_INSTALL_PATH):
	-mkdir -p $@

.PHONY install: all
	install -m 0755 $(OUT) $(LIBRARY_INSTALL_PATH)
	install -m 0644 $(INCLUDE_PATH)/dropbox.h $(INCLUDE_INSTALL_PATH)

uninstall: 
	rm -rf $(LIBRARY_INSTALL_PATH)/libdropbox.so $(INCLUDE_INSTALL_PATH)/dropbox.h

clean:
	rm -rf $(OBJ) $(OUT) $(EXAMPLE)

rebuild: clean all

