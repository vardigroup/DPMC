# ADDMC (Algebraic Decision Diagram Model Counter)
- ADDMC computes exact literal-weighted model counts of formulas in conjunctive normal form
  - Developer: [Vu Phan][url_homepage_vp]

<!-- ####################################################################### -->

## Installation
See `INSTALL.md`

<!-- ####################################################################### -->

## Examples

### Showing command-line options
#### Command
```bash
./addmc -h
```
#### Output
```
==================================================================
ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
==================================================================

Usage:
  addmc [OPTION...]

 Optional options:
  -h, --hi      help information
      --cf=arg  cnf file path (to use stdin, type: --cf=-)        Default arg: -
      --wf=arg  weight format in cnf file:
           1    UNWEIGHTED                                        
           2    MINIC2D                                           
           3    CACHET                                            Default arg: 3
           4    MCC                                               
      --ow=arg  output weight format:
           2    MINIC2D                                           
           3    CACHET                                            Default arg: 3
           4    MCC                                               
      --jf=arg  jt file path (to use stdin, type: --jf=-)         Default arg: (empty)
      --jw=arg  jt wait duration before jt planner is killed      Default arg: 10 (seconds)
      --pf=arg  performance factor                                Default arg: 1e-20
      --of=arg  output format:
           0    WEIGHTED_FORMULA
           1    JOIN_TREE
           2    MODEL_COUNT (using input jt file if provided)     Default arg: 2
      --ch=arg  clustering heuristic:
           1    MONOLITHIC                                        
           2    LINEAR                                            
           3    BUCKET_LIST                                       
           4    BUCKET_TREE                                       
           5    BOUQUET_LIST                                      
           6    BOUQUET_TREE                                      Default arg: 6
      --cv=arg  cluster variable order heuristic (negate to invert):
           1    APPEARANCE                                        
           2    DECLARATION                                       
           3    RANDOM                                            
           4    MCS                                               
           5    LEXP                                              Default arg: +5
           6    LEXM                                              
           7    MIN_FILL                                          
      --dv=arg  diagram variable order heuristic (negate to invert):
           1    APPEARANCE                                        
           2    DECLARATION                                       
           3    RANDOM                                            
           4    MCS                                               Default arg: +4
           5    LEXP                                              
           6    LEXM                                              
           7    MIN_FILL                                          
      --rs=arg  random seed                                       Default arg: 10
      --vl=arg  verbosity level:
           0    solution only                                     Default arg: 0
           1    parsed info as well                               
           2    cnf clauses and literal weights as well           
           3    input lines as well                               
```

### Computing join tree given cnf file
#### Command
```bash
./addmc --cf ../examples/phi.cnf --of 1
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c ==================================================================

c Process ID of this main program:
c pid 380447

c Reading CNF formula...

c Computing output...
c ------------------------------------------------------------------
p jt 5 6 17
7 6 e
12 7 e 5
10 1 2 e
9 4 e
8 5 e
13 8 e
14 9 13 e 4
15 10 14 e 3 1
11 3 e
16 11 e 2
17 12 15 16 e
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.017          
c ==================================================================
```

### Computing model count given cnf file
#### Command
```bash
./addmc --cf ../examples/phi.cnf
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c ==================================================================

c Process ID of this main program:
c pid 380477

c Reading CNF formula...

c Computing output...
c ------------------------------------------------------------------
s wmc 0.03125
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.017          
c ==================================================================
```

### Computing model count given cnf file and join tree from jt file
#### Command
```bash
./addmc --cf ../examples/phi.cnf --jf ../examples/phi.jt.htb
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c ==================================================================

c Process ID of this main program:
c pid 127493

c Reading CNF formula...

c Reading join tree...
c ------------------------------------------------------------------
c After 0.000000 seconds, finished reading last legal join tree ending on or before line 29 with ADD width 2

c Computing output...
c ------------------------------------------------------------------
s wmc 0.03125
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.008
c ==================================================================
```

### Computing model count given cnf file and join tree from stdin (`--jt -`)
#### Command
```bash
./addmc --cf ../examples/phi.cnf --jf - < ../examples/phi.jt.htb
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c ==================================================================

c Process ID of this main program:
c pid 127523

c Reading CNF formula...

c Reading join tree...
c Constructed timer with 10.000000-second timeout
c ==================================================================
c Getting join tree from stdin with 10.000000-second timeout... (end input with 'Enter' then 'Ctrl d')
c ******************************************************************
c MY_WARNING: alarm should have been disarmed; disarming alarm now
c ******************************************************************
c Disarmed alarm
c ------------------------------------------------------------------
c After 0.000000 seconds, finished reading last legal join tree ending on or before line 29 with ADD width 2
c Getting join tree from stdin: done
c ==================================================================

c Computing output...
c ------------------------------------------------------------------
s wmc 0.03125
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.01
c ==================================================================
```

<!-- ####################################################################### -->

## Acknowledgment
- Adnan Darwiche and Umut Oztok: [miniC2D][url_minic2d]
- David Kebo: [CUDD visualization][url_cudd_visualization]
- Fabio Somenzi: [CUDD package][url_cudd_package]
- Henry Kautz and Tian Sang: [Cachet][url_cachet]
- Jarryd Beck: [cxxopts][url_cxxopts]
- Johannes Fichte and Markus Hecher: [model counting competition][url_mcc]
- Lucas Tabajara: [RSynth][url_rsynth]
- Rob Rutenbar: [CUDD tutorial][url_cudd_tutorial]
- Vu Phan: [ADDMC][url_addmc]

<!-- ####################################################################### -->

[url_homepage_jd]:http://jmd11.web.rice.edu/
[url_homepage_mv]:https://www.cs.rice.edu/~vardi/
[url_homepage_vp]:https://vuphan314.github.io/

[url_addmc]:https://github.com/vardigroup/ADDMC
[url_cachet]:https://www.cs.rochester.edu/u/kautz/Cachet/Model_Counting_Benchmarks/index.htm
[url_cudd_package]:https://github.com/ivmai/cudd
[url_cudd_tutorial]:http://db.zmitac.aei.polsl.pl/AO/dekbdd/F01-CUDD.pdf
[url_cudd_visualization]:http://davidkebo.com/cudd#cudd6
[url_cxxopts]:https://github.com/jarro2783/cxxopts
[url_mcc]:https://mccompetition.org/2020/mc_format
[url_minic2d]:http://reasoning.cs.ucla.edu/minic2d
[url_rsynth]:https://bitbucket.org/lucas-mt/rsynth
[url_tensororder]:https://github.com/vardigroup/TensorOrder
