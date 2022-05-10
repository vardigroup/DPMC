# ProCount (Projected Model Counting)
ProCount computes weighted projected model counts of formulas in conjunctive normal form (CNF)
- The ProCount framework runs in two phases:
  - Planning phase: [LG](./lg) or [HTB](./htb) constructs a graded join tree of a CNF formula
  - Execution phase: [DMC](./dmc) computes the projected model count of the CNF formula using the constructed graded join tree
- Developers:
  - Jeffrey Dudek
  - Vu Phan

<!-- ####################################################################### -->

## [Releases](https://github.com/vardigroup/DPMC/releases)
- 2021/05/23: [v2.0.0](https://github.com/vardigroup/DPMC/releases/tag/v2.0.0)
  - SAT-2021 paper: **ProCount: Weighted Projected Model Counting with Graded Project-Join Trees**
    - Authors: Jeffrey Dudek, Vu Phan, Moshe Vardi
- 2020/07/20: [v1.0.0](https://github.com/vardigroup/DPMC/releases/tag/v1.0.0)
  - CP-2020 paper: **DPMC: Weighted Model Counting by Dynamic Programming on Project-Join Trees**
    - Authors: Jeffrey Dudek, Vu Phan, Moshe Vardi

<!-- ####################################################################### -->

## [Example files](./examples)

<!-- ####################################################################### -->

## [Experimental evaluation](./eval)

<!-- ####################################################################### -->

## [Acknowledgment](./ACKNOWLEDGMENT.md)
