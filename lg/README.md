# LG

## Description

A tool for using tree decompositions to construct join trees.

## Running with Singularity

Because of the variety of dependencies used in the various graph decomposition tools, it is recommended to use Singularity to run LG.

### Installation

The container can be built with the following commands:
```
sudo make lg.sif
```

### Running

Once the container has been built, LG can be run with the following command:

Once LG and FlowCutter has been built, LG can be run with the following command:
```bash
./lg.sif "/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" <../examples/phi.wpcnf
```

On this command, output is:
```
c pid 267669
c min degree heuristic
c outputing bagsize 2
p jt 6 5 10
6 1 e 2 4
7 4 5 e 3 5
8 2 e 6
9 7 8 3 e 1
10 6 9 e
c joinTreeWidth 2
c seconds 0.0115054
=
c status 2 1621823976810
c min shortcut heuristic
c run with 0.0/0.1/0.2 min balance and node_min_expansion in endless loop with varying seed
^C
```
Note that LG is an anytime algorithm, so it prints multiple join trees to STDOUT separated by '='.
The pid of the tree decomposition solver is given in the first comment line (`c pid`) and can be killed to stop the tree decomposition solver.

LG can also be run using Tamaki or htd as the tree decomposition solver as follows:
```bash
./lg.sif "java -classpath /solvers/TCS-Meiji -Xmx25g -Xms25g -Xss1g tw.heuristic.MainDecomposer -s 1234567 -p 100" <../examples/phi.wpcnf
./lg.sif "/solvers/htd-master/bin/htd_main -s 1234567 --opt width --iterations 0 --strategy challenge --print-progress --preprocessing full" <../examples/phi.wpcnf
```
Note that "Xmx25g" and "Xms25g" refers to the amount of memory given to the JVM in the tree decomposition solver (in this case, 25GB).
Upon an error message that begins `OpenJDK 64-Bit Server VM warning: INFO: os::commit_memory`, reduce 25 to a smaller number.

## Running without Singularity

The prerequisites to install LG, at minimum, are:
* make
* g++
* boost (graph and system)

LG can then be built with the following command:
```
make
```

To be useful, a tree decomposition solver must also be installed.
Options include:
* Tamaki; compiled using the `heuristic` instructions [here](solvers/TCS-Meiji).
* FlowCutter; compiled using the instructions [here](solvers/flow-cutter-pace17).
* htd; compiled using the instructions [here](solvers/htd-master).
