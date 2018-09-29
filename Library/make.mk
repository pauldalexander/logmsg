################################################################################
#
#	Makefile for liblogmsg.so - Release version
#
#	Note: The following environment variables should be set prior to
#	invoking this Makefile: 
#
#	${PREFIX}          - Path to installation folder.
#
#	${PKG_CONFIG_PATH} - Search path for .pc files.
#
################################################################################

INTERFACE_DIR=../Interface/include
INC_DIR=include
SRC_DIR=src

LIB_NAME=liblogmsg

OUT_FILE=$(LIB_NAME).so

INCLUDES=-I$(INTERFACE_DIR) -I$(INC_DIR)

SRC_FILES=$(SRC_DIR)/logmsg.c \

CC = gcc

CFLAGS=-g -O2 -Wall -fPIC -std=gnu99
CFLAGS+=$(INCLUDES)

LDFLAGS=-shared -Wl,--as-needed

LIBS=

all: $(OUT_FILE)

$(OUT_FILE): $(SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC_FILES) $(LIBS) -o $(OUT_FILE)

install:
	make-pc-file "logmsg" $(PREFIX) "Logging facility for program debugging" 1.0.0
	mkdir -p $(PREFIX)/lib
	cp $(OUT_FILE) $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	cp $(INTERFACE_DIR)/*.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib/pkgconfig
	cp $(LIB_NAME).pc $(PREFIX)/lib/pkgconfig
	
clean:
	$(RM) *.o *.so *.pc *.h
	
uninstall:
	$(RM) $(PREFIX)/lib/$(OUT_FILE)
	$(RM) $(PREFIX)/include/logmsg.h
	$(RM) $(PREFIX)/lib/pkgconfig/$(LIB_NAME).pc
	
.PHONY: install clean uninstall

