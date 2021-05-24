# Model Counting Competition MC-2021: DPMC and ProCount
- This repository implements DPMC (published at CP 2020) and ProCount (published at SAT 2021).
- DPMC competed in Tracks 1, 2, and 4 (unprojected counting), and ProCount competed in Track 3 (projected counting).
- The preprocessor [pmc v1.1](./bin/pmc) was [downloaded](http://www.cril.univ-artois.fr/KC/pmc.html) and used on all tracks.

<!-- ####################################################################### -->

### Downloads
- [Description](https://github.com/vardigroup/DPMC/releases/download/mc-2021/dpmc.pdf)
- [Solver](https://github.com/vardigroup/DPMC/releases/download/mc-2021/dpmc.zip)

<!-- ####################################################################### -->

### Building
```bash
make dpmc.zip
```

<!-- ####################################################################### -->

## Examples
The path to a benchmark must be given as the first positional argument `$1` (`stdin` unsupported).

<!-- ----------------------------------------------------------------------- -->

### Taurus cluster at TU Dresden
- The argument `--maxrss` specifies the RAM cap in GB.
- The argument `--tmpdir` specifies the temporary directory.

#### Track 1
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=mc --mp=1 $1
```

#### Track 2
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=wmc --mp=0 $1
```

#### Track 3
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=pmc --mp=1 $1
```

#### Track 4
```bash
bin/driver.py --cluster=tu --maxrss=4 --tmpdir=bin --task=mc --mp=0 $1
```

<!-- ----------------------------------------------------------------------- -->

### StarExec cluster at U of Iowa
- The environment variable `$STAREXEC_MAX_MEM` specifies the RAM cap in MB.
- The second positional argument `$2` specifies the temporary directory.

#### Track 1
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_1 $1 $2
```

#### Track 2
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_2 $1 $2
```

#### Track 3
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_3 $1 $2
```

#### Track 4
```bash
export STAREXEC_MAX_MEM=4000 && bin/starexec_run_4 $1 $2
```

<!-- ----------------------------------------------------------------------- -->

### Notes
Arguments and environment variables not mentioned above are ignored.
