export TENSORORDER_DIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

pys := $(shell find src -name "*.py")

tensor.sif: Singularity $(pys)
	singularity build -F tensor.sif Singularity

clean:
	rm -f tensor.sif
