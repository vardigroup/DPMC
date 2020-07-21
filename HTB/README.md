# `HTB` (Heuristic Tree Builder)
- `HTB` constructs join trees of formulas in conjunctive normal form for model counting
  - Developer: [Vu Phan][url_homepage_vp]

<!-- ####################################################################### -->

## Installation (Linux)

### Prerequisites
- g++ 6.4.0
- make 3.82
- unzip 6.00

### Command
```bash
./SETUP.sh
```

<!-- ####################################################################### -->

## Examples

### Showing command-line options
#### Command
```bash
./htb
```
#### Output
```
c ==================================================================
c HTB: Heuristic Tree Builder
c ==================================================================

Usage:
  htb [OPTION...]

 Required options:
      --cf arg  cnf file path (input)

 Optional options:
      --wf=arg  weight format in cnf file:
           1    UNWEIGHTED                                        
           2    MINIC2D                                           
           3    CACHET                                            Default arg: 3
           4    MCC                                               
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
./htb --cf ../examples/phi.cnf
```
#### Output
```
c ==================================================================
c HTB: Heuristic Tree Builder
c Version v0, released on 2020/05/29
c ==================================================================

c Process ID of this main program:
c pid 23176

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
c seconds                       0.008
c ==================================================================
```

<!-- ####################################################################### -->

## Acknowledgment
- Adnan Darwiche and Umut Oztok: [miniC2D][url_minic2d]
- Henry Kautz and Tian Sang: [Cachet][url_cachet]
- Jarryd Beck: [cxxopts][url_cxxopts]
- Johannes Fichte and Markus Hecher: [model counting competition][url_mcc]
- Lucas Tabajara: [RSynth][url_rsynth]
- Vu Phan: [ADDMC][url_addmc]

<!-- ####################################################################### -->

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
