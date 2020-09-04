# `DPMC` (Dynamic Programming for Model Counting)
- `DPMC` computes exact literal-weighted model counts of formulas in conjunctive normal form (CNF)
  - The `DPMC` framework runs in two phases:
    - Planning phase: `HTB` or `lg` constructs a join tree of a CNF formula
    - Execution phase: `DMC` or `tensor`computes the model count of the CNF formula using its join tree
  - Developers:
    - [Jeffrey M. Dudek][url_homepage_jd]
    - [Vu H. N. Phan][url_homepage_vp]
- CP-2020 paper: [**DPMC: Weighted Model Counting by Dynamic Programming on Project-Join Trees**](https://arxiv.org/abs/2008.08748)
  - Authors:
    - [Jeffrey M. Dudek][url_homepage_jd]
    - [Vu H. N. Phan][url_homepage_vp]
    - [Moshe Y. Vardi][url_homepage_mv]

<!-- ####################################################################### -->

## [Releases with Assets for Download](https://github.com/vardigroup/DPMC/releases)

- 2020/07/20: [v1.0.0](https://github.com/vardigroup/DPMC/releases/tag/v1.0.0)
  - Assets:
    - Experimental benchmarks, data, code, and figures: [experiments.zip](https://github.com/vardigroup/DPMC/releases/download/v1.0.0/experiments.zip)

<!-- ####################################################################### -->

## Planning phase: constructing join trees of CNF formulas
- [`HTB` (heuristic tree builder)](./HTB)
- [`lg` (line graph)](./lg)

## Execution phase: computing model counts of CNF formulas using their join trees
- [`DMC` (diagram model counter)](./DMC)
- [`tensor`](./tensor)

<!-- ####################################################################### -->

## Acknowledgment
- Adnan Darwiche and Umut Oztok: [miniC2D][url_minic2d]
- David Kebo: [CUDD visualization][url_cudd_visualization]
- Fabio Somenzi: [CUDD package][url_cudd_package]
- Henry Kautz and Tian Sang: [Cachet][url_cachet]
- Jarryd Beck: [cxxopts][url_cxxopts]
- Jeffrey Dudek: [TensorOrder][url_tensororder]
- Johannes Fichte and Markus Hecher: [model counting competition][url_mcc]
- Lucas Tabajara: [RSynth][url_rsynth]
- Rob Rutenbar: [CUDD tutorial][url_cudd_tutorial]
- Vu Phan: [ADDMC][url_addmc]

<!-- ####################################################################### -->

[url_homepage_jd]:http://jmd11.web.rice.edu/
[url_homepage_mv]:https://www.cs.rice.edu/~vardi/
[url_homepage_vp]:https://vuphan314.github.io/

[url_addmc]:https://github.com/vardigroup/ADDMC
[url_cachet]:https://www.cs.rochester.edu/u/kautz/Cachet/Model_Counting_Benchmarks/index.htm
[url_cudd_package]:https://github.com/ivmai/cudd
[url_cudd_tutorial]:http://db.zmitac.aei.polsl.pl/AO/dekbdd/F01-CUDD.pdf
[url_cudd_visualization]:http://davidkebo.com/cudd#cudd6
[url_cxxopts]:https://github.com/jarro2783/cxxopts
[url_mcc]:https://mccompetition.org/2020/mc_format
[url_minic2d]:http://reasoning.cs.ucla.edu/minic2d
[url_rsynth]:https://bitbucket.org/lucas-mt/rsynth
[url_tensororder]:https://github.com/vardigroup/TensorOrder
