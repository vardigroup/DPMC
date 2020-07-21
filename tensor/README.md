# Tensor
A Python 3 tool for valuating project-join trees with tensors.

## Running with Singularity
### Building the container
The container can be built with the following commands (make requires root to build the Singularity container):
```
sudo make
```

### Usage
Once built, example usage is:
```
[PLANNER] | ./tensor.sif --formula="../benchmarks/cubic_vertex_cover/cubic_vc_50_0.cnf" --timeout="100"
```
