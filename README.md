# DPMC/ProCount: Dynamic Programming for (Projected) Model Counting
DPMC/ProCount computes weighted (projected) model counts of formulas in conjunctive normal form (CNF)
- The DPMC/ProCount framework runs in two phases:
  - Planning phase: [LG](./lg) or [HTB](./htb) constructs a (graded) project-join tree of a CNF formula
  - Execution phase: [DMC](./dmc) computes the weighted (projected) model count of the formula using the (graded) project-join tree
- Developers:
  - Jeffrey Dudek
  - Vu Phan

<!-- ####################################################################### -->

## [Releases](https://github.com/vardigroup/DPMC/releases)
- 2021/05/25: [mc-2021](https://github.com/vardigroup/DPMC/releases/tag/mc-2021) [![DOI](https://zenodo.org/badge/280443175.svg)](https://zenodo.org/badge/latestdoi/280443175)
  - [Model Counting Competition MC-2021](./mcc)
- 2021/05/23: [v2.0.0](https://github.com/vardigroup/DPMC/releases/tag/v2.0.0)
  - SAT-2021 paper: [**ProCount: Weighted Projected Model Counting with Graded Project-Join Trees**](https://jmd11.web.rice.edu/papers/sat21_procount.pdf)
    - Authors: Jeffrey M. Dudek, Vu H. N. Phan, Moshe Y. Vardi
- 2020/07/20: [v1.0.0](https://github.com/vardigroup/DPMC/releases/tag/v1.0.0)
  - CP-2020 paper: [**DPMC: Weighted Model Counting by Dynamic Programming on Project-Join Trees**](https://arxiv.org/abs/2008.08748)
    - Authors: Jeffrey M. Dudek, Vu H. N. Phan, Moshe Y. Vardi

<!-- ####################################################################### -->

## [Example files](./examples)

<!-- ####################################################################### -->

## [Acknowledgment](./ACKNOWLEDGMENT.md)
