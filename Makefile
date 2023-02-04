###############################################
# Definitions
###############################################
CC = gcc
EXEC = cube
SRC_DIR = src
INC_DIR = include
CFLAGS = -Wall -Wno-maybe-uninitialized -I$(INC_DIR) -O3 -std=gnu99
<<<<<<< HEAD
LDFLAGS = -lncurses -ltinfo -lm
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

###################################################
# release                                         #
###################################################
PREFIX = /usr

.PHONY: install
install: $(EXEC) 
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/$(EXEC)

.PHONY: uninstall
uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(EXEC)
	# remove configs too if necessary
