# `DMC` (Diagram Model Counter)
- `DMC` computes exact literal-weighted model counts of formulas in conjunctive normal form
  - `DMC` uses algebraic decision diagrams as the primary data structure
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
./dmc
```
#### Output
```
c ==================================================================
c DMC: Diagram Model Counter
c ==================================================================

Usage:
  dmc [OPTION...]

 Required options:
      --cf arg  cnf file path (input)
      --jf arg  jt file path (to use stdin, type: '--jf -')

 Optional options:
      --wf=arg  weight format in cnf file:
           1    UNWEIGHTED                                        
           2    MINIC2D                                           
           3    CACHET                                            Default arg: 3
           4    MCC                                               
      --jw=arg  jt wait duration before jt planner is killed      Default arg: 10 (seconds)
      --pf=arg  performance factor                                Default arg: 1e-20
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

### Computing model count given join tree (from lg) and formula (from cnf file)
#### Command
```bash
cnf="../examples/50-10-1-q.cnf" && ../lg/lg.sif "/solvers/htd-master/bin/htd_main -s 1234567 --opt width --iterations 0 --strategy challenge --print-progress --preprocessing full" < $cnf | ./dmc --cf=$cnf --jf=- --pf=1e-3
```
#### Output
```
c ==================================================================
c DMC: Diagram Model Counter
c ==================================================================

c Process ID of this main program:
c pid 134276

c Reading CNF formula...

c Reading join tree...
c Constructed timer with 10.000000-second timeout
c ==================================================================
c Getting join tree from stdin with 10.000000-second timeout... (end input with 'Enter' then 'Ctrl d')
c ------------------------------------------------------------------
c After 0.130000 seconds, finished reading last legal join tree ending on or before line 278 with ADD width 15
c Remaining duration: 9.886000 seconds
c Requested new timeout: 32.768 seconds -- request declined
c ------------------------------------------------------------------
c After 0.138000 seconds, finished reading last legal join tree ending on or before line 554 with ADD width 14
c Remaining duration: 9.877000 seconds
c Requested new timeout: 16.384 seconds -- request declined
c ------------------------------------------------------------------
c After 0.153000 seconds, finished reading last legal join tree ending on or before line 833 with ADD width 12
c Remaining duration: 9.863000 seconds
c Requested new timeout: 4.096 seconds -- request granted
c ------------------------------------------------------------------
c Received SIGALRM after 4.249000 seconds since the main programm started
c Successfully killed planner process with PID 134282
c ------------------------------------------------------------------
c After 4.253000 seconds, finished reading last legal join tree ending on or before line 833 with ADD width 12
c Getting join tree from stdin: done
c ==================================================================

c Computing output...
c ------------------------------------------------------------------
s wmc 0.530158
c ------------------------------------------------------------------

c ==================================================================
c seconds                       4.351
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
