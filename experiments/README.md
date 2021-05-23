# Experiments (Linux)

<!-- ####################################################################### -->

## Benchmarks

### Downloading archives to this dir (`experiments`)
- [ProCount](https://github.com/vardigroup/DPMC/releases/download/v2.0.0/benchmarksDpmc.zip)
- [D4P & projMC](https://github.com/vardigroup/DPMC/releases/download/v2.0.0/benchmarksD4.zip)
- [reSSAT](https://github.com/vardigroup/DPMC/releases/download/v2.0.0/benchmarksSsat.zip)

### Extracting downloaded archives into the same new dir `experiments/benchmarks`
```bash
unzip ./benchmarksDpmc.zip
unzip ./benchmarksD4.zip
unzip ./benchmarksSsat.zip
```

### Files
- `.wpcnf` files (weighted projected CNF formulas):
  - Dir [`./benchmarks/waps/wpcnf`](https://github.com/meelgroup/WAPS#benchmarks)
  - Dir [`./benchmarks/bird/wpcnf`](https://github.com/meelgroup/ApproxMC#how-to-cite)
- For each `.wpcnf` file, we made:
  - A `.cnf` file, a `.var` file, and a `.weight` file for D4P & projMC
  - A `.sdimacs` file for reSSAT

<!-- ####################################################################### -->

## Tools
- `lg.sif`: ProCount's (main) planner [LG](../lg)
- `htb`: ProCount's (alternate) planner [HTB](../htb)
- `dmc`: ProCount's executor [DMC](../dmc)
- `LgDmc.sh`: [ProCount model counter (script to pipe LG's output into DMC's input)](./LgDmc.sh)
- `d4`: [d4p & projMC model counters (corrected version provided by author)](https://github.com/vardigroup/DPMC/releases/download/v2.0.0/d4)
- `abc`: [reSSAT model counter](https://github.com/nianzelee/ssatABC)
- `cryptominisat5_amd64_linux_static`: [SAT solver](https://github.com/msoos/cryptominisat)

### ProCount, using planner LG (with tree decomposer FlowCutter) and executor DMC (with heuristic MCS)
```bash
wpcnfFile=benchmarks/bird/wpcnf/blasted_case102.no_w.wpcnf && ../lg/lg.sif "/solvers/flow-cutter-pace17/flow_cutter_pace17 -s 1234567 -p 100" < $wpcnfFile | ../dmc/dmc --cf=$wpcnfFile --jf=-
```
#### Output solution and time
```
s wmc 0.123694
c seconds                       0.107
```

### D4P
According to the author (Jean-Marie Lagniez), D4P requires the option `-pv=NO` to correctly compute weighted projected model counts
```bash
cnfFile=benchmarks/bird/cnf/blasted_case102.no_w.cnf && d4 -mc $cnfFile -fpv=${cnfFile//cnf/var} -wFile=${cnfFile//cnf/weight} -pv=NO
```
#### Output solution and time
```
s 0.123694
c Final time: 0.000810
```

### projMC
```bash
cnfFile=benchmarks/bird/cnf/blasted_case102.no_w.cnf && d4 -emc $cnfFile -fpv=${cnfFile//cnf/var} -wFile=${cnfFile//cnf/weight}
```
#### Output solution and time
```
s 0.123694
c Final time: 0.021300
```

### reSSAT
```bash
sdimacsFile=benchmarks/bird/sdimacs/blasted_case102.no_w.sdimacs && abc -c "ssat $sdimacsFile"
```
#### Output solution and time
```
> Upper bound = 1.236941e-01
> Time        =     0.01 sec
```

<!-- ####################################################################### -->

## Data
- Download [archive](https://github.com/vardigroup/DPMC/releases/download/v2.0.0/data.zip) to this dir (`experiments`)
- Extract downloaded archive into dir `experiments/data`

### ProCount, using planner LG (with tree decomposer FlowCutter) and executor DMC (with heuristic MCS)
- File `./data/solving/dpmc/lg/flow/mcs/203.out`: line 1 shows the options used with the solver (as a Python dictionary); the remaining lines are stdout
- File `./data/solving/dpmc/lg/flow/mcs/203.log`: stderr
- File `./data/solving/dpmc/lg/flow/mcs0/203.out`: line 1 is dummy; the remaining lines are `key:value` pairs parsed from stdout and stderr (this postprocessing step was excluded from the solving time of every solver)
  - Key `count`: the weighted projected model count of the benchmark, from stdout
  - Key `toolTime`: the time reported by the solver, from stdout (`toolTime` can be null in case of timeout or error)
  - Key `wallTime`: the time reported by the Linux utility `/usr/bin/time`, from stderr
  - Key `time`: `toolTime` if `toolTime` is not null else `wallTime`

### D4P
Dir `./data/solving/d4p0`

### projMC
Dir `./data/solving/projmc0`

### reSSAT
Dir `./data/solving/ssat0`

### nestHDB & ProCount: 90 de-weighted WAPS benchmarks with 100-second timeout (preliminary results)
Dir `./data/camera/`

<!-- ####################################################################### -->

## [Jupyter notebook](./procount.ipynb)
- Near the end, there are 7 figures (4 in the main paper and 3 in the supplement)
- Run all cells again to re-generate these figures from dir `./data`
- Some Python lines for running experiments (e.g., submitting Slurm jobs and parsing stdout/stderr) have been commented out

<!-- ####################################################################### -->

## Notes on timing
Recall that `time` is the time a solver spent to solve a benchmark
- If a figure's time axis is in logarithmic scale, then we used max(`time`, 1e-3) for display purposes
- Otherwise, we used the original `time` recorded
