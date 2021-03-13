# DMC (diagram model counter)
DMC computes exact literal-weighted model counts of formulas in conjunctive normal form using algebraic decision diagrams
- Developer: [Vu Phan](https://vuphan314.github.io)

<!-- ####################################################################### -->

## Installation (Linux)

### Prerequisites
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
sudo make dmc.sif
```
#### Without Singularity (quick)
```bash
make dmc
```

<!-- ####################################################################### -->

## Examples
If you use Singularity, then replace `./dmc --cf=$cnfFile` with `singularity run --bind="/:/host" ./dmc.sif --cf=/host$(realpath $cnfFile)` in the following commands.

### Showing command-line options
#### Command
```bash
./dmc
```
#### Output
```
Diagram Model Counter (reads jointree from STDIN)
Usage:
  dmc [OPTION...]

      --cf arg  CNF file path; string (REQUIRED)
      --wf arg  weight format in CNF file: c/CACHET, m/MINIC2D, p/PROJECTED,
                u/UNWEIGHTED, w/WEIGHTED; string (default: p)
      --ps arg  planning strategy: f/FIRST_JOINTREE, t/TIMED_JOINTREES;
                string (default: f)
      --jw arg  jointree wait seconds before killing planner [with ps_arg ==
                t]; float (default: 0.5)
      --tc arg  thread count, or '0' for hardware_concurrency value; int
                (default: 1)
      --dp arg  diagram package: c/CUDD, s/SYLVAN; string (default: c)
      --rs arg  random seed; int (default: 2020)
      --dv arg  diagram var order: 0/RANDOM, 1/DECLARED, 2/MOST_CLAUSES,
                3/MINFILL, 4/MCS, 5/LEXP, 6/LEXM (negatives for inverse orders);
                int (default: 4)
      --sv arg  slice var order [with tc_arg != 1 and dp_arg == c]: 0/RANDOM,
                1/DECLARED, 2/MOST_CLAUSES, 3/MINFILL, 4/MCS, 5/LEXP, 6/LEXM,
                7/BIGGEST_NODE, 8/HIGHEST_NODE (negatives for inverse
                orders); int (default: 7)
      --mm arg  max megabytes for unique table and cache table combined; int
                (default: 1024)
      --ir arg  init ratio for tables [with dp_arg == s]:
                log2(max_size/init_size); int (default: 0)
      --tr arg  table ratio [with dp_arg == s]:
                log2(unique_table_size/cache_table_size); int (default: 0)
      --mp arg  multiple precision [with dp_arg == s]: 0, 1; int (default: 0)
      --lc arg  log counting [with dp_arg == c]: 0, 1; int (default: 0)
      --jp arg  join priority: a/ARBITRARY_PAIR, b/BIGGEST_PAIR,
                s/SMALLEST_PAIR; string (default: s)
      --vl arg  verbosity level: 0/SOLUTION, 1/PARSED_COMMAND,
                2/CLAUSES_AND_WEIGHTS, 3/INPUT_LINES; int (default: 1)
```

### Computing model count given CNF file and jointree file
#### Command
```bash
./dmc --tc=0 --lc=1 --cf=../examples/s27_3_2.pcnf <../examples/s27_3_2.jt
```
#### Output
```
c processing command-line options...
c cnfFile                   ../examples/s27_3_2.pcnf
c weightFormat              PROJECTED
c planningStrategy          FIRST_JOINTREE
c threadCount               8
c diagramPackage            CUDD
c randomSeed                2020
c diagramVarOrder           MCS
c sliceVarOrder             BIGGEST_NODE
c maxMegabytes              1024
c logCounting               1
c joinPriority              SMALLEST_PAIR

c processing CNF formula...
c declaredVarCount          20
c apparentVarCount          20
c declaredClauseCount       43
c apparentClauseCount       43

c procressing jointree...
c ==================================================================
c getting jointree from STDIN... (end input with 'Enter' then 'Ctrl d')
c processed jointree ending on line 18
c jointreeWidth             9
c jointreeSeconds           0.00124136
c MY_WARNING: failed to kill planner process with PID 299074
c getting jointree from STDIN: done
c ==================================================================
c declaredNodeCount         56

c computing output...
c sliceWidth                7
c threadMegabytes           128
c thread     0 | assignment {    -3    -7    -5 }
c thread     0 | time     0 | count 0.0831402       | ln(count) -2.48723
c thread     1 | assignment {     3    -7    -5 }
c thread     1 | time     0 | count 0.0191838       | ln(count) -3.95369
c thread     2 | assignment {    -3     7    -5 }
c thread     2 | time     0 | count 0.0105061       | ln(count) -4.5558
c thread     3 | assignment {     3     7    -5 }
c thread     3 | time     0 | count 0.0020279       | ln(count) -6.20076
c thread     4 | assignment {    -3    -7     5 }
c thread     4 | time     0 | count 0.117334        | ln(count) -2.14273
c thread     5 | assignment {     3    -7     5 }
c thread     5 | time     0 | count 0.291191        | ln(count) -1.23378
c thread     6 | assignment {    -3     7     5 }
c thread     6 | time     0 | count 0               | ln(count) -inf
c thread     7 | assignment {     3     7     5 }
c thread     7 | time     0 | count 0.0268819       | ln(count) -3.6163
c ------------------------------------------------------------------
s pmc                       0.550265
c ln(pmc)                   -0.597356
c ------------------------------------------------------------------
c seconds                   0.058
```

### Computing model count given jointree (from `lg`) and formula (from CNF file)
#### Command
```bash
cnfFile="../examples/overflow.cnf" && ../lg/lg.sif "/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < $cnfFile | ./dmc --cf=$cnfFile --wf=u --dp=s --mp=1
```
#### Output
```
c processing command-line options...
c cnfFile                   ../examples/overflow.cnf
c weightFormat              UNWEIGHTED
c planningStrategy          FIRST_JOINTREE
c threadCount               1
c diagramPackage            SYLVAN
c randomSeed                2020
c diagramVarOrder           MCS
c maxMegabytes              1024
c initRatio                 0
c tableRatio                0
c multiplePrecision         1
c joinPriority              SMALLEST_PAIR

c processing CNF formula...
c declaredVarCount          1025
c apparentVarCount          1
c declaredClauseCount       1
c apparentClauseCount       1

c procressing jointree...
c ==================================================================
c getting jointree from STDIN... (end input with 'Enter' then 'Ctrl d')
c processed jointree ending on line 6
c jointreeWidth             1
c jointreeSeconds           0.00400174
c killed planner process with PID 228414
c getting jointree from STDIN: done
c ==================================================================
c declaredNodeCount         2

c computing output...
c ------------------------------------------------------------------
s mc                        179769313486231590772930519078902473361797697894230657273430081157732675805500963132708477322407536021120113879871393357658789768814416622492847430639474124377767893424865485276302219601246094119453082952085005768838150682342462881473913110540827237163350510684586298239947245938479716304835356329624224137216
c stold(mc)                 inf
c stoll(mc)                 -9223372036854775808
c ------------------------------------------------------------------
c seconds                   0.139
```
