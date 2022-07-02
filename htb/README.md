# HTB (heuristic tree builder)
HTB constructs (graded) join trees for XOR-CNF formulas.

--------------------------------------------------------------------------------

## Installation (Linux)

### Prerequisites
#### External libraries
- boost 1.66
- g++ 11.2
- gmp 6.2
- make 4.2
#### [Included libraries](../addmc/libraries/)
- [cxxopts 2.2](https://github.com/jarro2783/cxxopts)

### Command
```bash
make htb
```

--------------------------------------------------------------------------------

## Examples

### Showing options
#### Command
```bash
./htb
```
#### Output
```
Heuristic Tree Builder
Usage:
  htb [OPTION...]

      --cf arg  CNF file path; string (required)
      --pc arg  projected counting (graded join tree): 0, 1; int (default: 0)
      --rs arg  random seed; int (default: 0)
      --cv arg  cluster var order: 0/RANDOM, 1/DECLARATION, 2/MOST_CLAUSES, 3/MIN_FILL, 4/MCS, 5/LEX_P, 6/LEX_M
                (negatives for inverse orders); int (default: 5)
      --ch arg  clustering heuristic: bel/BUCKET_ELIM_LIST, bet/BUCKET_ELIM_TREE, bml/BOUQUET_METHOD_LIST,
                bmt/BOUQUET_METHOD_TREE; string (default: bmt)
      --vc arg  verbose CNF processing: 0, 1, 2, 3; int (default: 0)
      --vs arg  verbose solving: 0, 1, 2; int (default: 0)
  -h            help
```

### Finding graded join tree (for projected counting) given CNF formula from file
#### Command
```bash
./htb --cf=../examples/s27_3_2.cnf --pc=1
```
#### Output
```
c htb process:
c pid 133981

c processing CNF formula...

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
c joinTreeWidth                 11
c seconds                       0.002
```
