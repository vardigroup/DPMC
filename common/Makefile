# OPT=-Ofast# provided from command line
# LINK=-static

################################################################################

HPP_DIR = src/interface
CPP_DIR = src/implementation

################################################################################

LIB_DIR = ./lib# unzipped from lib.zip by INSTALL.sh

SYLVAN_DIR = $(LIB_DIR)/sylvan
SYLVAN_INCLUSIONS = -I$(SYLVAN_DIR)/src
SYLVAN_LINKS = -L$(SYLVAN_DIR)/build/src -lsylvan -lpthread -lgmp -lgmpxx -lm -lrt# order matters

CUDD_DIR = $(LIB_DIR)/cudd
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LINKS = -L$(CUDD_DIR)/cudd/.libs -lcudd

################################################################################

O_DIR = obj

_OBJ = util.o graph.o formula.o join.o phase.o
_OBJ_HTB = $(_OBJ) main_htb.o
_OBJ_DMC = $(_OBJ) main_dmc.o

OBJ_HTB = $(patsubst %, $(O_DIR)/%, $(_OBJ_HTB))
OBJ_DMC = $(patsubst %, $(O_DIR)/%, $(_OBJ_DMC))

################################################################################

BIN_DIR = .

HTB = htb
DMC = dmc

HTB_BIN = $(BIN_DIR)/$(HTB)
DMC_BIN = $(BIN_DIR)/$(DMC)

################################################################################

GCC = g++ -g# debugging information
GCC_FLAGS = $(OPT) $(LINK) -I./$(HPP_DIR) -I$(LIB_DIR) $(CUDD_INCLUSIONS) $(SYLVAN_INCLUSIONS) -fconcepts -std=c++14

################################################################################

$(HTB): $(OBJ_HTB)
	mkdir -p $(BIN_DIR) && $(GCC) -o $(HTB_BIN) $^ $(GCC_FLAGS) $(CUDD_LINKS) $(SYLVAN_LINKS) && echo

$(DMC): $(OBJ_DMC)
	mkdir -p $(BIN_DIR) && $(GCC) -o $(DMC_BIN) $^ $(GCC_FLAGS) $(CUDD_LINKS) $(SYLVAN_LINKS) && echo

$(O_DIR)/%.o: $(CPP_DIR)/%.cpp $(HPP_DIR)/%.hpp
	mkdir -p $(O_DIR) && $(GCC) -c -o $@ $< $(GCC_FLAGS) && echo

.PHONY: all clean

all: $(HTB) $(DMC)

clean:
	rm -rf $(O_DIR) $(HTB_BIN) $(DMC_BIN)
