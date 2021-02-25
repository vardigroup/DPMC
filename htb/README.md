# HTB (Heuristic Tree Builder)
HTB constructs join trees of formulas in conjunctive normal form for model counting
- Developer: Vu Phan

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
  - [cudd 3.0](https://github.com/ivmai/cudd)
  - [cxxopts 2.2](https://github.com/jarro2783/cxxopts)
  - [sylvan 1.5](https://trolando.github.io/sylvan)

### Command
#### With Singularity 3.5 (slow)
```bash
sudo make
```
#### Without Singularity (quick)
```bash
./SETUP.sh
```

<!-- ####################################################################### -->

## Examples
If you use Singularity, then replace `./htb` with `singularity run --bind="/:/host" ./htb.sif` in the following commands.

### Showing command-line options
#### Command
```bash
./htb
```
#### Output
```
HTB: Heuristic Tree Builder

Usage:
  htb [OPTION...]

 Required options:
      --cf arg  cnf file path (input)

 Optional options:
      --wf=arg  weight format in cnf file:
           1    UNWEIGHTED                                        
           2    MINIC2D                                           
           3    CACHET                                            
           4    WCNF                                              
           5    WPCNF                                             Default arg: 5
      --ch=arg  clustering heuristic:
           3    BUCKET_LIST                                       
           4    BUCKET_TREE                                       
           5    BOUQUET_LIST                                      
           6    BOUQUET_TREE                                      Default arg: 6
      --cv=arg  cluster var order heuristic (negate to invert):
           1    APPEARANCE                                        
           2    DECLARATION                                       
           3    RANDOM                                            
           4    MCS                                               
           5    LEXP                                              Default arg: +5
           6    LEXM                                              
           7    MINFILL                                           
      --rs=arg  random seed                                       Default arg: 2020
      --vl=arg  verbosity level:
           0    solution only                                     
           1    parsed info as well                               Default arg: 1
           2    cnf clauses and literal weights as well           
           3    input lines as well                               
```

### Computing join tree given cnf file
#### Command
```bash
./htb --cf=../examples/s27_3_2.wpcnf
```
#### Output
```
c argv: ./htb --cf=../examples/s27_3_2.wpcnf

c processing command-line options...
c cnfFilePath                   ../examples/s27_3_2.wpcnf
c weightFormat                  WPCNF
c clustering                    BOUQUET_TREE
c clusterVarOrder               LEXP
c inverseClusterVarOrder        0
c randomSeed                    2020

c processing cnf formula...
c declaredVarCount              20
c apparentVarCount              20
c declaredClauseCount           43
c apparentClauseCount           43

c computing output...
c ------------------------------------------------------------------
p jt 20 43 58
44 43 e
45 13 39 40 41 42 44 e 20
46 34 35 36 37 38 e 19
47 29 30 31 32 33 e 18
48 22 27 28 45 46 47 e 17
49 9 10 11 12 14 48 e 12
50 24 e
51 1 2 25 26 50 e 16
52 21 23 49 51 e 15
53 3 4 5 52 e 8
54 15 16 17 18 53 e 11
55 6 7 8 19 20 54 e 13 10 9 14
56 55 e
57 56 e 4 1 6 2 5 7 3
58 57 e
c joinTreeWidth 11
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.003
c ==================================================================
```
