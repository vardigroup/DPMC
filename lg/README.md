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
./lg.sif "/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" <../examples/s27_3_2.wpcnf
```

On this command, output is:
```
c pid 271711
c min degree heuristic
c outputing bagsize 9
p jt 20 43 56
44 25 26 24 e 16
45 44 1 2 3 5 4 e 8
46 6 15 17 19 20 16 18 e 13
47 46 7 8 e 10
48 34 35 36 37 38 e 19
49 48 39 40 41 42 43 e 20
50 29 30 31 32 33 e 18
51 49 50 13 14 12 e 12
52 47 51 10 11 9 e 11
53 52 21 27 28 e 17
54 45 53 e 9
55 54 22 23 e 14 15
56 55 e 1 2 3 4 5 6 7
c joinTreeWidth 9
c seconds 0.011639
=
c status 9 1621825834767
c min shortcut heuristic
c outputing bagsize 8
p jt 20 43 57
44 25 26 24 e 16
45 34 35 36 37 38 e 19
46 39 40 41 42 43 e 20
47 29 30 31 32 33 e 18
48 45 46 47 27 28 e 17
49 48 13 14 12 e 12
50 49 10 11 9 15 17 16 e 11
51 50 6 19 20 18 21 e 13
52 51 7 8 e 10
53 52 22 23 e 14
54 44 53 4 e 15
55 54 3 5 e 9
56 55 1 2 e 8
57 56 e 1 2 3 4 5 6 7
c joinTreeWidth 8
c seconds 0.0117734
=
c status 8 1621825834767
c run with 0.0/0.1/0.2 min balance and node_min_expansion in endless loop with varying seed
^C
```
Note that LG is an anytime algorithm, so it prints multiple join trees to STDOUT separated by '='.
The pid of the tree decomposition solver is given in the first comment line (`c pid`) and can be killed to stop the tree decomposition solver.

LG can also be run using Tamaki or htd as the tree decomposition solver as follows:
```bash
./lg.sif "java -classpath /solvers/TCS-Meiji -Xmx25g -Xms25g -Xss1g tw.heuristic.MainDecomposer -s 1234567 -p 100" <../examples/s27_3_2.wpcnf
./lg.sif "/solvers/htd-master/bin/htd_main -s 1234567 --opt width --iterations 0 --strategy challenge --print-progress --preprocessing full" <../examples/s27_3_2.wpcnf
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
