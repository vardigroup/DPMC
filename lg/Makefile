appname := lg

CXX := g++
CXXFLAGS := -std=c++17 -O3 -DNDEBUG -I ./src -pedantic
LDLIBS := -lboost_system -pthread
link := -static

srcfiles := $(shell find . -name "*.cc" -not -path "./solvers/*")
headers := $(shell find . -name "*.h" -not -path "./solvers/*")
objects := $(patsubst ./%.cpp, ./%.o, $(patsubst ./%.cc, ./%.o, $(srcfiles)))

all: build/$(appname)

build/$(appname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o build/$(appname) $(objects) $(LDLIBS) $(link)

build/.depend: $(srcfiles)
	mkdir -p build
	$(CXX) $(CXXFLAGS) -MM $^>>./build/.depend;

clean:
	rm -f $(objects) $(appname).sif
	rm -rf ./build

dist-clean: clean
	rm -f *~ ./build/.depend

$(appname).sif: Singularity $(srcfiles) $(headers)
	singularity build $(appname).sif Singularity

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), $(appname).sif)
include build/.depend
endif
endif
