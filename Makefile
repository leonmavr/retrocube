###############################################
# Definitions
###############################################
CC = gcc
EXEC = cube
SRC_DIR = src
INC_DIR = include
# where to store the mesh (text) files -
# set it from the command line if you want another location
PREFIX = /usr
CFG_DIR = $(PREFIX)/share/retrocube
CFLAGS = -Wall -Wno-stringop-truncation -Wno-maybe-uninitialized -I$(INC_DIR)\
	-std=gnu99 -O3 -DCFG_DIR=$(CFG_DIR)
LDFLAGS = -lm
SOURCES = $(wildcard $(SRC_DIR)/*.c) \
	main.c
OBJECTS = $(SOURCES:%.c=%.o)
MKDIR = mkdir -p
CP = cp -r
RM = rm -rf


###############################################
# Compilation
###############################################
all: $(EXEC)

$(EXEC): $(OBJECTS) cfg
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

###############################################
# Commands (phony targets)
###############################################
.PHONY: cfg
cfg:
	# copy config files into CFG_DIR
	$(MKDIR) $(CFG_DIR)
	$(CP) mesh_files/*.scl $(CFG_DIR)

.PHONY: clean
clean:
	$(RM) $(OBJECTS)
	$(RM) $(EXEC)
