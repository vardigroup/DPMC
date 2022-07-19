# Tensor

A Python 3 tool for valuating project-join trees with tensors.

## Running with Singularity

### Building the container

The container can be built with the following commands (`make` requires root to build the Singularity container):
```bash
sudo make
```

### Usage

Once built, example usage is:
```bash
cnfFile="../examples/50-10-1-q.cnf" && ../lg/lg.sif "/solvers/flow-cutter-pace17/flow_cutter_pace17 -p 100" <$cnfFile | ./tensor.sif --formula=$cnfFile --timeout=100
```

Output:
```
Parsed join tree with tensor width 16
Join Tree Time: 0.0211584
Tree decomposition stream ended.
Count: 0.5330335096711298
Parse Formula Time: 0.002299070358276367
Parse Join Tree Time: 0.006467103958129883
Execution Time: 0.09494352340698242
Total Time: 0.1037135124206543
````
