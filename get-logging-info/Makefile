################################################################################
#
#	Makefile for get-logging-info
#
################################################################################

SRC_DIR=.

PROGRAM_NAME=get-logging-info

OUT_FILE=$(PROGRAM_NAME)

SRC_FILES=$(SRC_DIR)/main.c

CC = gcc

CFLAGS=-g -O0 -Wall -std=c99

LDFLAGS=

LIBS=

$(OUT_FILE): $(SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC_FILES) $(LIBS) -o $(OUT_FILE)
	
install:
	
clean:
	$(RM) $(OUT_FILE) *.o
	
.PHONY: install clean

