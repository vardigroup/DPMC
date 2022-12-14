## in dir src/: file common.hh is included by files dmc.hh and htb.hh

## GXX = g++-11
GXX = g++

# opt = -Ofast # be careful with inf and nan
ASSEMBLY_OPTIONS = -g -std=c++2a -Wno-register $(opt)

# link = -static # beware of segfault with std::thread
LINK_OPTIONS = -lgmpxx -lgmp -lpthread $(link) # order matters

################################################################################

CXXOPTS_TARGET = libraries/cxxopts/include/cxxopts.hpp

CUDD_DIR = libraries/cudd
CUDD_LIB_DIR = $(CUDD_DIR)/cudd/.libs
CUDD_TARGET = $(CUDD_LIB_DIR)/libcudd.a
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LINKS = -L$(CUDD_LIB_DIR) -lcudd

SYLVAN_DIR = libraries/sylvan
SYLVAN_BUILD_DIR = $(SYLVAN_DIR)/build
SYLVAN_LIB_DIR = $(SYLVAN_BUILD_DIR)/src
SYLVAN_TARGET = $(SYLVAN_LIB_DIR)/libsylvan.a
SYLVAN_INCLUSIONS = -I$(SYLVAN_DIR)/src
SYLVAN_LINKS = -L$(SYLVAN_LIB_DIR) -lsylvan

CMSAT_DIR = libraries/cryptominisat
CMSAT_BUILD_DIR = $(CMSAT_DIR)/build
CMSAT_LIB_DIR = $(CMSAT_BUILD_DIR)/lib
CMSAT_TARGET = $(CMSAT_LIB_DIR)/libcryptominisat5.a
CMSAT_INCLUSIONS = -I$(CMSAT_BUILD_DIR)/include
CMSAT_LINKS = -L$(CMSAT_LIB_DIR) -lcryptominisat5

COLAMD_DIR = libraries/colamd
COLAMD_LINKS = -L$(COLAMD_DIR) -lcolamd
################################################################################

DMC_OBJECTS = common.o dmc.o
HTB_OBJECTS = common.o htb.o

.ONESHELL: # for all targets

dmc: $(DMC_OBJECTS)
	$(GXX) -o dmc $(DMC_OBJECTS) $(CUDD_LINKS) $(SYLVAN_LINKS) $(CMSAT_LINKS) $(COLAMD_LINKS) $(LINK_OPTIONS)

htb: $(HTB_OBJECTS)
	$(GXX) -o htb $(HTB_OBJECTS) $(LINK_OPTIONS)

dmc.o: src/dmc.cc src/dmc.hh src/common.hh $(CXXOPTS_TARGET) $(CUDD_TARGET) $(SYLVAN_TARGET) $(CMSAT_TARGET)
	$(GXX) src/dmc.cc -c $(ASSEMBLY_OPTIONS) $(CUDD_INCLUSIONS) $(SYLVAN_INCLUSIONS) $(CMSAT_INCLUSIONS)

htb.o: src/htb.cc src/htb.hh src/common.hh $(CXXOPTS_TARGET)
	$(GXX) src/htb.cc -c $(ASSEMBLY_OPTIONS)

common.o: src/common.cc src/common.hh
	$(GXX) src/common.cc -c $(ASSEMBLY_OPTIONS)

$(CUDD_TARGET): $(shell find $(CUDD_DIR)/cudd -name "*.c" -o -name "*.h") $(shell find $(CUDD_DIR)/cplusplus -name "*.cc" -o -name "*.hh")
	cd $(CUDD_DIR)
	autoreconf
	./configure --silent --enable-obj
	make -s

$(SYLVAN_TARGET):
	mkdir -p $(SYLVAN_BUILD_DIR)
	cd $(SYLVAN_BUILD_DIR)
	cmake .. -DBUILD_SHARED_LIBS=off
	make -s

$(CMSAT_TARGET):
	mkdir -p $(CMSAT_BUILD_DIR)
	cd $(CMSAT_BUILD_DIR)
	cmake .. -DSTATICCOMPILE=on -DONLY_SIMPLE=ON -DNOZLIB=ON -DNOM4RI=ON -DNOSQLITE=ON
	make -s

all: dmc htb

cudd: $(CUDD_TARGET)

sylvan: $(SYLVAN_TARGET)

cryptominisat: $(CMSAT_TARGET)

clean:
	rm -f *.o dmc htb

clean-dmc:
	rm -f $(DMC_OBJECTS) dmc

clean-htb:
	rm -f $(HTB_OBJECTS) htb

clean-cudd:
	cd $(CUDD_DIR) && git clean -xdf

clean-sylvan:
	rm -rf $(SYLVAN_BUILD_DIR)

clean-cryptominisat:
	rm -rf $(CMSAT_BUILD_DIR)

clean-libraries: clean-cudd clean-sylvan clean-cryptominisat

clean-all: clean clean-libraries
