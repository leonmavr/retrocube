###############################################
# Definitions
###############################################
CC = gcc
EXEC = cube
SRC_DIR = src
INC_DIR = include
CFLAGS = -Wall -I$(INC_DIR) -O3 -std=c99
LDFLAGS = -lncurses -lm
SOURCES = $(wildcard $(SRC_DIR)/*.c) \
	main.c
OBJECTS = $(SOURCES:%.c=%.o)
RM = rm -rf


###############################################
# Compilation 
###############################################
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) 
	$(RM) $(EXEC)
