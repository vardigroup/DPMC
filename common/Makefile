# OPT=-Ofast# must not use inf or nan
# LINK=-static# segfault with std::thread

################################################################################

CUDD_DIR = lib/cudd
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LINKS = -L$(CUDD_DIR)/cudd/.libs -lcudd

SYLVAN_DIR = lib/sylvan
SYLVAN_INCLUSIONS = -I$(SYLVAN_DIR)/src
SYLVAN_LINKS = -L$(SYLVAN_DIR)/build/src -lsylvan -lpthread

ASSEMBLY_OPTIONS = $(OPT) -g -Ilib -Isrc -fconcepts -std=c++2a -Wno-register
LINK_OPTIONS = $(LINK) -lgmpxx -lgmp -lm -lrt# order matters

################################################################################

OBJECTS = util.o logic.o
DMC_OBJECTS = $(OBJECTS) dmc.o
HTB_OBJECTS = $(OBJECTS) htb.o

dmc: $(DMC_OBJECTS)
	g++ -o dmc $(DMC_OBJECTS) $(LINK_OPTIONS) $(CUDD_LINKS) $(SYLVAN_LINKS) && echo

dmc.o: src/dmc.cc src/dmc.hh src/logic.hh src/util.hh lib
	g++ -o dmc.o src/dmc.cc -c $(ASSEMBLY_OPTIONS) $(CUDD_INCLUSIONS) $(SYLVAN_INCLUSIONS) && echo

htb: $(HTB_OBJECTS)
	g++ -o htb $(HTB_OBJECTS) $(LINK_OPTIONS) && echo

htb.o: src/htb.cc src/htb.hh src/logic.hh src/util.hh lib
	g++ -o htb.o src/htb.cc -c $(ASSEMBLY_OPTIONS) && echo

logic.o: src/logic.cc src/logic.hh src/util.hh lib
	g++ -o logic.o src/logic.cc -c $(ASSEMBLY_OPTIONS) && echo

util.o: src/util.cc src/util.hh lib
	g++ -o util.o src/util.cc -c $(ASSEMBLY_OPTIONS) && echo

lib: lib.zip
	unzip -qq -n -D lib.zip && \
	cd $(CUDD_DIR) && \
	./configure --quiet --prefix=$(CURDIR)/$(CUDD_DIR)/INSTALLED --enable-obj && \
	make --quiet && \
	make --quiet install && \
	cd ../../$(SYLVAN_DIR) && \
	mkdir -p build && \
	cd build && \
	cmake -DCMAKE_INSTALL_PREFIX=../INSTALLED -DBUILD_SHARED_LIBS=OFF .. && \
	make --quiet && \
	make --quiet install && \
	echo

.PHONY: all clean

all: dmc htb

clean:
	rm -rf $(OBJECTS) dmc.o htb.o dmc htb && echo
