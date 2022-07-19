# OPT=-Ofast# must not use inf or nan
# LINK=-static# segfault with std::thread

################################################################################

HPP_DIR = src/interface
CPP_DIR = src/implementation

################################################################################

CUDD_DIR = lib/cudd
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LINKS = -L$(CUDD_DIR)/cudd/.libs -lcudd

SYLVAN_DIR = lib/sylvan
SYLVAN_INCLUSIONS = -I$(SYLVAN_DIR)/src
SYLVAN_LINKS = -L$(SYLVAN_DIR)/build/src -lsylvan -lpthread -lgmp -lgmpxx -lm -lrt# order matters

################################################################################

O_DIR = obj

_OBJ = util.o graph.o formula.o join.o phase.o
_OBJ_DMC = $(_OBJ) main_dmc.o
_OBJ_HTB = $(_OBJ) main_htb.o

OBJ_DMC = $(patsubst %, $(O_DIR)/%, $(_OBJ_DMC))
OBJ_HTB = $(patsubst %, $(O_DIR)/%, $(_OBJ_HTB))

################################################################################

GCC = g++ -g# debugging information
GCC_FLAGS = $(OPT) $(LINK) -I./$(HPP_DIR) -Ilib $(CUDD_INCLUSIONS) $(SYLVAN_INCLUSIONS) -fconcepts -std=c++14

################################################################################

dmc: $(OBJ_DMC)
	$(GCC) -o dmc $^ $(GCC_FLAGS) $(CUDD_LINKS) $(SYLVAN_LINKS) && echo

htb: $(OBJ_HTB)
	$(GCC) -o htb $^ $(GCC_FLAGS) $(CUDD_LINKS) $(SYLVAN_LINKS) && echo

$(O_DIR)/%.o: $(CPP_DIR)/%.cpp $(HPP_DIR)/%.hpp lib
	mkdir -p $(O_DIR) && $(GCC) -c -o $@ $< $(GCC_FLAGS) && echo

lib: lib.zip
	unzip -qq -n -D lib.zip && \
	cd $(CUDD_DIR) && \
	./configure --quiet --enable-obj && \
	make --quiet && \
	cd ../../$(SYLVAN_DIR) && \
	mkdir -p build && \
	cd build && \
	cmake -DBUILD_SHARED_LIBS=OFF .. && \
	make --quiet && \
	echo

.PHONY: all clean

all: dmc htb

clean:
	rm -rf $(O_DIR) dmc htb
