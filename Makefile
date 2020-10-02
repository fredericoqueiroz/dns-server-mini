SHELL:=/bin/bash -O globstar

BIN_NAME_1 = servidor_dns

SRC_NAME_1=udpDnsServer

CC = gcc
CFLAGS = -Wall -Wextra -g

BIN_DIR = .
SRC_DIR = ./src
OBJ_DIR = ./src/obj
LIB_DIR = ./src/lib

LIB_SRC=$(wildcard $(LIB_DIR)/*.c)

.PHONY: all

all: setup server

setup:
	mkdir -p $(OBJ_DIR)
	
server: $(SRC_NAME_1).o
	$(CC) $(LIB_SRC) $(OBJ_DIR)/$(SRC_NAME_1).o -lpthread -o $(BIN_NAME_1)

$(SRC_NAME_1).o:
	$(CC) -c $(SRC_DIR)/$(SRC_NAME_1).c -o $(OBJ_DIR)/$(SRC_NAME_1).o $(CFLAGS)

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/$(BIN_NAME_1)
	rmdir $(OBJ_DIR)
