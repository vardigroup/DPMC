# HTB (heuristic tree builder)
HTB constructs jointrees of formulas in conjunctive normal form
- Developer: [Vu Phan](https://vuphan314.github.io)

<!-- ####################################################################### -->

## Installation (Linux)

### Prerequisites
- boost 1.66
- cmake 3.11
- g++ 6.4
- gmp 6.2
- make 3.82
- unzip 6.0
- already included in `../common/lib.zip`:
  - [cudd 3.0.0](https://github.com/ivmai/cudd/tree/cudd-3.0.0)
  - [cxxopts 2.2.1](https://github.com/jarro2783/cxxopts/tree/v2.2.1)
  - [sylvan 1.5.0](https://github.com/trolando/sylvan/tree/v1.5.0)

### Command
#### With Singularity 3.5 (slow)
```bash
sudo make htb.sif
```
#### Without Singularity (quick)
```bash
make htb
```

<!-- ####################################################################### -->

## Examples
If you use Singularity, then replace `./htb --cf=$cnfFile` with `singularity run --bind="/:/host" ./htb.sif --cf=/host$(realpath $cnfFile)` in the following commands.

### Showing command-line options
#### Command
```bash
./htb
```
#### Output
```
Heuristic Tree Builder
Usage:
  htb [OPTION...]

      --cf arg  CNF file path; string (REQUIRED)
      --wf arg  weight format in CNF file: c/CACHET, m/MINIC2D, p/PROJECTED,
                u/UNWEIGHTED, w/WEIGHTED; string (default: p)
      --rs arg  random seed; int (default: 2020)
      --cv arg  cluster var order: 0/RANDOM, 1/DECLARED, 2/MOST_CLAUSES,
                3/MINFILL, 4/MCS, 5/LEXP, 6/LEXM (negatives for inverse orders);
                int (default: 5)
      --ch arg  clustering heuristic: bel/BE_LIST, bet/BE_TREE, bml/BM_LIST,
                bmt/BM_TREE; string (default: bmt)
      --vl arg  verbosity level: 0/SOLUTION, 1/PARSED_COMMAND,
                2/CLAUSES_AND_WEIGHTS, 3/INPUT_LINES; int (default: 1)
```

### Computing jointree given CNF file
#### Command
```bash
./htb --cf=../examples/s27_3_2.pcnf
```
#### Output
```
c PID of this HTB process:
c pid 40790

c processing command-line options...
c cnfFile                   ../examples/s27_3_2.pcnf
c weightFormat              PROJECTED
c randomSeed                2020
c clusterVarOrder           LEXP
c clusteringHeuristic       BM_TREE

c processing CNF formula...
c declaredVarCount          20
c apparentVarCount          20
c declaredClauseCount       43
c apparentClauseCount       43

c computing output...
c ------------------------------------------------------------------
p jt 20 43 59
46 15 e
44 7 e
45 3 6 8 e
47 18 19 20 44 45 e 10
48 9 10 11 16 17 46 47 e 13
49 1 2 4 5 48 e 9
50 21 22 23 49 e 14
51 12 13 14 50 e 11
52 24 25 26 51 e 8 16
53 27 28 52 e 15
54 39 40 41 42 43 e 20
55 34 35 36 37 38 e 19
56 29 30 31 32 33 53 54 55 e 12 17 18
57 56 e
58 57 e 4 5 2 3 1 6 7
59 58 e
c ------------------------------------------------------------------
c jointreeWidth             11
c seconds                   0.014
```
