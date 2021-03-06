#####################################################################

# Makefile for stl2gdml
# Cory R. Thornsberry
# updated: Dec. 6th, 2018

#####################################################################

# Set the binary install directory.
INSTALL_DIR = $(HOME)/bin

#####################################################################

CC = g++

#CFLAGS = -g -Wall -std=c++0x -Iinclude
CFLAGS = -Wall -O3 -std=c++0x -Iinclude
LDLIBS = 
LDFLAGS = 

# Directories
TOP_LEVEL = $(shell pwd)
INCLUDE_DIR = $(TOP_LEVEL)/include
SOURCE_DIR = $(TOP_LEVEL)/source
EXEC_DIR = $(TOP_LEVEL)/exec
OBJ_DIR = $(TOP_LEVEL)/obj

# Tools
ALL_TOOLS = stl2gdml
EXE_NAMES = $(addprefix $(EXEC_DIR)/, $(addsuffix .a, $(ALL_TOOLS)))
INSTALLED = $(addprefix $(INSTALL_DIR)/, $(ALL_TOOLS))

# Source files
SOURCES = facet gdmlEntry geantGdmlFile polySolid stlBlock threeTuple ySlice
OBJFILES = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(SOURCES)))

# List of directories to generate if they do not exist.
DIRECTORIES = $(EXEC_DIR) $(OBJ_DIR)

#####################################################################

all: $(DIRECTORIES) $(OBJFILES) $(EXE_NAMES)
#	Create all directories, make all objects, and link executable

.PHONY: $(ALL_TOOLS) $(INSTALLED) $(DIRECTORIES)

#####################################################################

$(DIRECTORIES): 
#	Make the default configuration directory
	@if [ ! -d $@ ]; then \
		echo "Making directory: "$@; \
		mkdir -p $@; \
	fi

#####################################################################

$(EXEC_DIR)/%.a: $(SOURCE_DIR)/%.cpp $(OBJFILES)
#	Compile C++ source files
	$(CC) $(CFLAGS) $< -o $@ $(OBJFILES) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
#	Compile C++ source files
	$(CC) $(CFLAGS) $< -c -o $@

#####################################################################

$(ALL_TOOLS):
	@echo " Installing "$(INSTALL_DIR)/$@
	@rm -f $(INSTALL_DIR)/$@
	@ln -s $(EXEC_DIR)/$@.a $(INSTALL_DIR)/$@

install: all $(ALL_TOOLS)
	@echo "Finished installing tools to "$(INSTALL_DIR)

########################################################################

$(INSTALLED):
	@rm -f $@

uninstall: $(INSTALLED)
	@echo "Finished uninstalling";

clean: uninstall
	@rm -f $(EXE_NAMES)
	@rm -f $(OBJFILES)
