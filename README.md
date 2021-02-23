# `ProCount` (Projected Model Counting)
- `ProCount` computes weighted projected model counts of formulas in conjunctive normal form (cnf)
- The `ProCount` framework runs in two phases:
  - Planning phase: [`htb`](./htb) or [`lg`](./lg) constructs a graded join tree of a cnf formula
  - Execution phase: [`dmc`](./dmc) computes the projected model count of the cnf formula using the constructed graded join tree
- Developers:
  - Jeffrey Dudek
  - Vu Phan

<!-- ####################################################################### -->

## [Example files](./examples)

<!-- ####################################################################### -->

## [Experimental evaluation](./experiments)

<!-- ####################################################################### -->

## Acknowledgment
- [ADDMC](https://github.com/vardigroup/ADDMC): Dudek, Phan, Vardi
- [BIRD](https://github.com/meelgroup/approxmc): Soos, Meel
- [Cachet](https://cs.rochester.edu/u/kautz/Cachet): Sang, Beame, Kautz
- [CUDD package](https://github.com/ivmai/cudd): Somenzi
- [CUDD visualization](https://davidkebo.com/cudd#cudd6): Kebo
- [cxxopts](https://github.com/jarro2783/cxxopts): Beck
- [DPMC](https://github.com/vardigroup/DPMC): Dudek, Phan, Vardi
- [FlowCutter](https://github.com/kit-algo/flow-cutter-pace17): Strasser
- [htd](https://github.com/mabseher/htd): Abseher, Musliu, Woltran
- [miniC2D](http://reasoning.cs.ucla.edu/minic2d): Oztok, Darwiche
- [Model-counting competition](https://mccompetition.org): Hecher, Fichte
- [SlurmQueen](https://github.com/Kasekopf/SlurmQueen): Dudek
- [Sylvan](https://trolando.github.io/sylvan): van Dijk
- [Tamaki](https://github.com/TCS-Meiji/PACE2017-TrackA): Tamaki
- [TensorOrder](https://github.com/vardigroup/TensorOrder): Dudek, Duenas-Osorio, Vardi
- [WAPS](https://github.com/meelgroup/WAPS): Gupta, Sharma, Roy, Meel
