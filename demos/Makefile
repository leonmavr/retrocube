###############################################
# Definitions
###############################################
CC = gcc
SRC_DIR = ../src
INC_DIR = ../include
DEMO_DIR = .
# where to store the mesh (text) files -
# set it from the command line if you want another location
PREFIX = /usr
CFG_DIR = $(PREFIX)/share/retrocube
CFLAGS = -Wall -Wno-stringop-truncation -Wno-maybe-uninitialized -I$(INC_DIR)\
	-std=gnu99 -O3 -DCFG_DIR=$(CFG_DIR)
LDFLAGS = -lm
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:%.c=%.o)
DEMOS = $(wildcard $(DEMO_DIR)/*.c)
DEMO_OBJECTS = $(DEMOS:%.c=%.o)
DEMO_EXECS = $(DEMOS:%.c=%)
MKDIR = mkdir -p
CP = cp -r
RM = rm -rf
# Usage: make print-<MY_VAR>
print-%  : ; @echo $* = $($*)

###############################################
# Compilation
###############################################
all: $(DEMO_EXECS) cfg


$(OBJECTS): $(SOURCES)
	$(foreach file, $(wildcard $(SRC_DIR)/*.c), $(CC) $(CFLAGS) -c $(file) -o $(basename $(file)).o;)

$(DEMO_OBJECTS): $(DEMOS) $(SOURCES)
	$(foreach file, $(wildcard $(DEMO_DIR)/*.c), $(CC) $(CFLAGS) -c $(file) -o $(basename $(file)).o;)

$(DEMO_EXECS): $(OBJECTS) $(DEMO_OBJECTS)
	$(foreach demo_obj, $(DEMO_OBJECTS), $(CC) $(OBJECTS) $(demo_obj) -o $(basename $(demo_obj)) $(LDFLAGS);)


###############################################
# Commands (phony targets)
###############################################
.PHONY: cfg
cfg:
	# copy config files into CFG_DIR
	$(MKDIR) $(CFG_DIR)
	$(CP) ../mesh_files/*.scl $(CFG_DIR)

.PHONY: clean
clean:
	$(RM) $(DEMO_OBJECTS)
	$(RM) $(DEMO_EXECS)
	$(RM) $(OBJECTS)
