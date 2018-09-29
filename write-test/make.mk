################################################################################
#
#	Makefile for write-test test program
#
################################################################################

SRC_DIR=.

PROGRAM_NAME=write-test

OUT_FILE=$(PROGRAM_NAME)

SRC_FILES=$(SRC_DIR)/main.c

CC = gcc

CFLAGS=-g -O2 -Wall -std=c99

LDFLAGS=

LIBS=

$(OUT_FILE): $(SRC_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC_FILES) $(LIBS) -o $(OUT_FILE)
	
clean:
	$(RM) $(OUT_FILE) *.o
	
install:
	
.PHONY: install clean

