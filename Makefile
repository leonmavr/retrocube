###############################################
# Definitions
###############################################
CC = gcc
EXEC = cube
SRC_DIR = src
INC_DIR = include
CFLAGS = -Wall -Wno-stringop-truncation -Wno-maybe-uninitialized -I$(INC_DIR) -O3 -std=gnu99
LDFLAGS = -lm
SOURCES = $(wildcard $(SRC_DIR)/*.c) \
	main.c
OBJECTS = $(SOURCES:%.c=%.o)
MKDIR = mkdir -p
CP = cp
RM = rm -rf
CFG_DIR = ${HOME}/.config/retrocube


###############################################
# Compilation 
###############################################
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 
	$(MKDIR) $(CFG_DIR)
	$(CP) ./mesh_files/*.scl $(CFG_DIR)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJECTS) 
	$(RM) $(EXEC)
