#pragma once

/* inclusions =============================================================== */

#include <boost/pending/disjoint_sets.hpp>

#include "../libraries/cxxopts/include/cxxopts.hpp"

#include "logic.hh"

/* uses ===================================================================== */

using cxxopts::value;

/* consts =================================================================== */

const string CLUSTER_VAR_OPTION = "cv";
const string CLUSTERING_HEURISTIC_OPTION = "ch";

/* classes ================================================================== */

class JoinComponent { // for projected model counting
public:
  Int varOrderHeuristic = MIN_INT;
  string clusteringHeuristic;
  vector<JoinNode*> subtrees; // R
  Set<Int> keptVars; // F

  Set<Int> projectableVars; // Z
  vector<Set<Int>> projectableVarSets; // Z_1..Z_m is a partition of Z
  vector<vector<JoinNode*>> nodeClusters; // kappa_0..kappa_m: kappa_0 is nodeClusters.back()

  JoinNonterminal* getComponentRoot();
  Set<Int> getNodeVars(const vector<JoinNode*>& nodes) const;
  vector<Int> getRestrictedVarOrder() const;

  JoinComponent(
    Int varOrderHeuristic,
    string clusteringHeuristic,
    const vector<JoinNode*>& subtrees,
    const Set<Int>& keptVars
  );
};

class JoinRootBuilder {
public:
  Set<Int> disjunctiveVars;
  vector<Set<Int>> disjunctiveVarSets; // clause index |-> vars
  vector<vector<Int>> clauseGroups; // group |-> clause indices

  void printDisjunctiveVarSets() const;
  void printClauseGroups() const;

  void setDisjunctiveVarSets(); // also sets disjunctiveVars
  void setClauseGroups();

  JoinNonterminal* buildRoot(Int varOrderHeuristic, string clusteringHeuristic) const;

  JoinRootBuilder();
};

class Planner { // abstract
public:
  JoinNonterminal* joinRoot = nullptr;

  bool usingTreeClustering; // as opposed to list clustering
  Int clusterVarOrderHeuristic;

  void printJoinTree() const;
  void outputJoinTree();

  virtual void setJoinTree() = 0;
};

class BucketPlanner : public Planner { // bucket elimination
public:
  void setJoinTree() override;

  BucketPlanner(bool usingTreeClustering, Int clusterVarOrderHeuristic);
};

class BouquetPlanner : public Planner { // Bouquet's Method
public:
  void setJoinTree() override;

  BouquetPlanner(bool usingTreeClustering, Int clusterVarOrderHeuristic);
};

class OptionDict {
public:
  string cnfFilePath;
  Int clusterVarOrderHeuristic;
  string clusteringHeuristic;

  static string helpClusteringHeuristic();
  void runCommand() const;

  OptionDict(int argc, char** argv);
};

/* global functions ========================================================= */

int main(int argc, char** argv);
