# Model Counting Competition 2024: DPMC and ProCount
- This repository implements DPMC (published at CP 2020) and ProCount (published at SAT 2021).
  - DPMC competes in Tracks 1 and 2 (unprojected counting).
  - ProCount competes in Tracks 3 and 4 (projected counting).
- The preprocessor [pmc v1.1](http://www.cril.univ-artois.fr/KC/pmc.html) is used on all tracks.

--------------------------------------------------------------------------------

## Downloading
[Solver](https://github.com/vardigroup/DPMC/releases/download/mc-2022/dpmc.zip)

--------------------------------------------------------------------------------

## Building
```bash
make dpmc.zip
```

--------------------------------------------------------------------------------

## Examples
The path to a benchmark must be given as the first positional argument `$1` (`stdin` is unsupported).

--------------------------------------------------------------------------------

### Taurus cluster at TU Dresden
- The argument `--maxrss` specifies the RAM cap in GB.
- The argument `--tmpdir` specifies the temporary directory.

#### Track 1
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp1 --task=mc --mp=1 instances/test.cnf

bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp1 --task=mc --mp=1 instances/mc2024_track1_029.cnf
```

#### Track 2
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp2 --task=wmc --mp=0 instances/test.cnf

bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp2 --task=wmc --mp=0 instances/mc2024_track2-random_029.cnf
```

#### Track 3
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp3 --task=pmc --mp=1 instances/test.cnf

bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp3 --task=pmc --mp=1 instances/mc2024_track3_131.cnf
```

#### Track 4
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp4 --task=pwmc --mp=0 instances/test.cnf

bin/driver.py --cluster=tu --maxrss=4 --tmpdir=tmp4 --task=pwmc --mp=0 instances/mc2024_track4_055.cnf
```

--------------------------------------------------------------------------------

### StarExec cluster at U of Iowa
- The environment variable `$STAREXEC_MAX_MEM` specifies the RAM cap in MB.
- The second positional argument `$2` specifies the temporary directory.

#### Track 1
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track1_pre1_mp1 instances/test.cnf tmp1

export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track1_pre1_mp1 instances/mc2024_track1_029.cnf tmp1
# c s log10-estimate 16.8941
# c seconds                       576.781
```

#### Track 2
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track2_pre1_mp0 instances/test.cnf tmp2

export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track2_pre1_mp0 instances/mc2024_track2-random_029.cnf tmp2
# c s log10-estimate 13.5112
# c seconds                       11.407
```

#### Track 3
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track3_pre1_mp1 instances/test.cnf tmp3

export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track3_pre1_mp1 instances/mc2024_track3_131.cnf tmp3
# c s log10-estimate 1.53148
# c seconds                       0.002
```

#### Track 4
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track4_pre1_mp0 instances/test.cnf tmp4

export STAREXEC_MAX_MEM=4000 && bin/starexec_run_track4_pre1_mp0 instances/mc2024_track4_055.cnf tmp4
# c s log10-estimate -78.945
# c seconds                       296.454
```

--------------------------------------------------------------------------------

## Notes
Arguments and environment variables not mentioned above are ignored.
