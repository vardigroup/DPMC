# DPMC/ProCount/DPO/DPER
- We provide four exact solvers that support XOR-CNF formulas.
  - DPMC solves *weighted model counting (WMC)*.
  - ProCount solves *weighted projected model counting (WPMC)*.
  - [DPO](https://arxiv.org/abs/2205.08632) solves *weighted SAT (WSAT)*, i.e., Boolean MPE.
  - [DPER](https://arxiv.org/abs/2205.09826) solves *exist-random SAT (ERSAT)*.
- Each of these four solvers is a combination of a planner and an executor.
  - A planner produces a **project-join tree** T from an XOR-CNF formula F.
  - An executor traverses T to computes a solution of F.
  - For WPMC and ERSAT, T must be **graded**.
- Two planners are available.
  - [HTB](./htb) uses constraint-programming heuristics.
  - [LG](./lg) uses tree decomposers.
- Two executors are available.
  - [DMC](./dmc) uses *algebraic decision diagrams (ADDs)*.
  - [Tensor](./tensor) uses tensors and only solves WMC on pure CNF.
- Developers:
  - Jeffrey Dudek: LG and Tensor
  - Vu Phan: HTB and DMC

--------------------------------------------------------------------------------

## [Releases](https://github.com/vardigroup/DPMC/releases)
- 2021/05/25: [mc-2021](https://github.com/vardigroup/DPMC/releases/tag/mc-2021) [![DOI](https://zenodo.org/badge/280443175.svg)](https://zenodo.org/badge/latestdoi/280443175)
  - [Model Counting Competition MC-2021](./mcc)
- 2021/05/23: [v2.0.0](https://github.com/vardigroup/DPMC/releases/tag/v2.0.0)
  - SAT-2021 paper: [**ProCount: Weighted Projected Model Counting with Graded Project-Join Trees**](https://jmd11.web.rice.edu/papers/sat21_procount.pdf)
    - Authors: Jeffrey M. Dudek, Vu H. N. Phan, Moshe Y. Vardi
- 2020/07/20: [v1.0.0](https://github.com/vardigroup/DPMC/releases/tag/v1.0.0)
  - CP-2020 paper: [**DPMC: Weighted Model Counting by Dynamic Programming on Project-Join Trees**](https://arxiv.org/abs/2008.08748)
    - Authors: Jeffrey M. Dudek, Vu H. N. Phan, Moshe Y. Vardi

--------------------------------------------------------------------------------

## Cloning this repository and its submodules
```bash
git clone --recursive https://github.com/vardigroup/DPMC
```

--------------------------------------------------------------------------------

## [Examples](./examples)

--------------------------------------------------------------------------------

## Acknowledgment
- [ADDMC](https://github.com/vardigroup/ADDMC): Dudek, Phan, Vardi
- [BIRD](https://github.com/meelgroup/approxmc): Soos, Meel
- [Cachet](https://cs.rochester.edu/u/kautz/Cachet): Sang, Beame, Kautz
- [CryptoMiniSat](https://github.com/msoos/cryptominisat): Soos
- [CUDD package](https://github.com/ivmai/cudd): Somenzi
- [CUDD visualization](https://davidkebo.com/cudd#cudd6): Kebo
- [cxxopts](https://github.com/jarro2783/cxxopts): Beck
- [FlowCutter](https://github.com/kit-algo/flow-cutter-pace17): Strasser
- [htd](https://github.com/mabseher/htd): Abseher, Musliu, Woltran
- [miniC2D](http://reasoning.cs.ucla.edu/minic2d): Oztok, Darwiche
- [Model Counting Competition](https://mccompetition.org): Hecher, Fichte
- [pmc](http://www.cril.univ-artois.fr/KC/pmc.html): Lagniez, Marquis
- [SlurmQueen](https://github.com/Kasekopf/SlurmQueen): Dudek
- [Sylvan](https://trolando.github.io/sylvan): van Dijk
- [Tamaki](https://github.com/TCS-Meiji/PACE2017-TrackA): Tamaki
- [TensorOrder](https://github.com/vardigroup/TensorOrder): Dudek, Duenas-Osorio, Vardi
- [WAPS](https://github.com/meelgroup/WAPS): Gupta, Sharma, Roy, Meel
