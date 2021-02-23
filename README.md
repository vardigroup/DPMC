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

## [Acknowledgment](./ACKNOWLEDGMENT.md)
