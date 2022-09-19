# HTB (Heuristic Tree Builder)
HTB constructs join trees of formulas in conjunctive normal form for model counting
- Developer: Vu Phan

--------------------------------------------------------------------------------

## Installation (Linux)

### Prerequisites
- boost 1.66
- cmake 3.11
- g++ 6.4
- gmp 6.2
- make 3.82
- unzip 6.0
- already included in `../addmc/lib.zip`:
  - [cudd 3.0](https://github.com/ivmai/cudd)
  - [cxxopts 2.2](https://github.com/jarro2783/cxxopts)
  - [sylvan 1.5](https://trolando.github.io/sylvan)

### Command
#### With Singularity 3.5 (slow)
```bash
sudo make htb.sif
```
#### Without Singularity (quick)
```bash
make htb
```

--------------------------------------------------------------------------------

## Examples
If you use Singularity, then replace `./htb --cf=$cnfFile` with `singularity run --bind="/:/host" ./htb.sif --cf=/host$(realpath $cnfFile)` in the following commands.

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

### Computing join tree given CNF file
#### Command
```bash
./htb --cf=../examples/phi.wpcnf
```
#### Output
```
c argv: ./htb --cf=../examples/phi.wpcnf

c processing command-line options...
c cnfFilePath                   ../examples/phi.wpcnf
c weightFormat                  WPCNF
c clustering                    BOUQUET_TREE
c clusterVarOrder               LEXP
c inverseClusterVarOrder        0
c randomSeed                    2020

c processing cnf formula...
c declaredVarCount              6
c apparentVarCount              6
c declaredClauseCount           5
c apparentClauseCount           5

c computing output...
c ------------------------------------------------------------------
p jt 6 5 15
11 1 e 2 4
12 11 e
7 4 e
8 5 e
13 7 8 e 3 5
6 3 e
9 2 e 6
10 9 e
14 6 10 e 1
15 12 13 14 e
c joinTreeWidth 2
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.001
c ==================================================================
```
