# Model Counting Competition 2022: DPMC and ProCount
- This repository implements DPMC (published at CP 2020) and ProCount (published at SAT 2021).
  - DPMC competed in Tracks 1 and 2 (unprojected counting).
  - ProCount competed in Tracks 3 and 4 (projected counting).
- The preprocessor [pmc v1.1](http://www.cril.univ-artois.fr/KC/pmc.html) was used on all tracks.

<!-- ####################################################################### -->

### Downloading
<!-- - [Description](https://github.com/vardigroup/DPMC/releases/download/mc-2022/dpmc.pdf) -->
- [Solver](https://github.com/vardigroup/DPMC/releases/download/mc-2022/dpmc.zip)

<!-- ####################################################################### -->

### Building
```bash
make dpmc.zip
```

<!-- ####################################################################### -->

## Examples
The path to a benchmark must be given as the first positional argument `$1` (`stdin` is unsupported).

<!-- ----------------------------------------------------------------------- -->

### Taurus cluster at TU Dresden
- The argument `--maxrss` specifies the RAM cap in GB.
- The argument `--tmpdir` specifies the temporary directory.

#### Track 1
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=mc --mp=1 test.cnf
```

#### Track 2
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=wmc --mp=0 test.cnf
```

#### Track 3
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=pmc --mp=1 test.cnf
```

#### Track 4
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=pwmc --mp=0 test.cnf
```

<!-- ----------------------------------------------------------------------- -->

### StarExec cluster at U of Iowa
- The environment variable `$STAREXEC_MAX_MEM` specifies the RAM cap in MB.
- The second positional argument `$2` specifies the temporary directory.

#### Track 1
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_1pre1 test.cnf $2
```

#### Track 2
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_2pre1 test.cnf $2
```

#### Track 3
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_3pre1 test.cnf $2
```

#### Track 4
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_4pre1 test.cnf $2
```

<!-- ----------------------------------------------------------------------- -->

### Notes
Arguments and environment variables not mentioned above are ignored.
