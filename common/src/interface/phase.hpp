#pragma once

/* inclusions *****************************************************************/

#include "join.hpp"

/* classes ********************************************************************/

class Tool { // for planning phase or execution phases
protected:
  JoinNonterminal *joinRoot = nullptr;

public:
  void printJoinTree(const Cnf &cnf) const;
};

class Planner : public Tool { // HTB builds join tree from cnf
protected:
  bool usingTreeClustering; // versus List
  VarOrderingHeuristic cnfVarOrderingHeuristic;
  bool inverseCnfVarOrdering;

  virtual void constructJoinTree(const Cnf &cnf) = 0; // handles cnf without empty clause
  void setJoinTree(const Cnf &cnf); // handles cnf with/without empty clause

public:
  JoinNonterminal *getJoinTree(const Cnf &cnf);
  void outputJoinTree(const Cnf &cnf);
};

class BucketPlanner : public Planner { // bucket elimination
protected:
  void constructJoinTree(const Cnf &cnf) override;

public:
  BucketPlanner(
    bool usingTreeClustering,
    VarOrderingHeuristic cnfVarOrderingHeuristic,
    bool inverseCnfVarOrdering
  );
};

class BouquetPlanner : public Planner { // Bouquet's Method
protected:
  void constructJoinTree(const Cnf &cnf) override;

public:
  BouquetPlanner(
    bool usingTreeClustering,
    VarOrderingHeuristic cnfVarOrderingHeuristic,
    bool inverseCnfVarOrdering
  );
};

class Executor : public Tool { // DMC computes model count from join tree
protected:
  VarOrderingHeuristic ddVarOrderingHeuristic;
  bool inverseDdVarOrdering;
  DdPackage ddPackage;

  Map<Int, Int> cnfVarToDdVarMap; // e.g. {42: 0, 13: 1}
  vector<Int> ddVarToCnfVarMap; // e.g. [42, 13], i.e. ddVarOrdering

  void orderDdVars(const Cnf &cnf); // modifies cnfVarToDdVarMap and ddVarToCnfVarMap
  Dd getClauseDd(const vector<Int> &clause) const;

  Dd countSubtree(JoinNode *joinNode, const Cnf &cnf, Set<Int> &projectedCnfVars); // modifies projectedCnfVars
  Float computeModelCount(const Cnf &cnf); // handles cnf without empty clause
  Float getModelCount(const Cnf &cnf); // handles cnf with/without empty clause

public:
  void outputModelCount(const Cnf &cnf);
  Executor(
    JoinNonterminal *joinRoot,
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};
