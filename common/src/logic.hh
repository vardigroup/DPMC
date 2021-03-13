#pragma once

/* inclusions =============================================================== */

#include "util.hh"

/* consts =================================================================== */

const string CNF_FILE_OPTION = "cf";
const string WEIGHT_FORMAT_OPTION = "wf";

const string UNWEIGHTED = "u";
const string MINIC2D = "m";
const string CACHET = "c"; // CACHET bug: '-1' weight
const string WEIGHTED = "w"; // 'w' line: trailing '0' is optional
const string PROJECTED = "p"; // 'vp' line: trailing '0' is optional
const map<string, string> WEIGHT_FORMATS = {
  {UNWEIGHTED, "UNWEIGHTED"},
  {MINIC2D, "MINIC2D"},
  {CACHET, "CACHET"},
  {WEIGHTED, "WEIGHTED"},
  {PROJECTED, "PROJECTED"}
};

const string BUCKET_LIST = "bel";
const string BUCKET_TREE = "bet";
const string BOUQUET_LIST = "bml";
const string BOUQUET_TREE = "bmt";
const map<string, string> CLUSTERING_HEURISTICS = {
  {BUCKET_LIST, "BE_LIST"},
  {BUCKET_TREE, "BE_TREE"},
  {BOUQUET_LIST, "BM_LIST"},
  {BOUQUET_TREE, "BM_TREE"}
};

const string PROBLEM_WORD = "p";
const string WEIGHTS_WORD = "weights"; // MINIC2D weights line
const string WEIGHT_WORD = "w"; // CACHET/WEIGHTED/PROJECTED weight lines
const string LINE_END_WORD = "0"; // required for clause lines and optional for weight lines

const string JT_WORD = "jt";
const string VAR_ELIM_WORD = "e";

/* classes for CNF formulas ================================================= */

class Assignment : public Map<Int, bool> { // partial var assignment
public:
  Assignment();
  Assignment(Int var, bool val);

  void printAssignment() const;
  static vector<Assignment> extendAssignments(const vector<Assignment>& assignments, Int var);
};

class Clause : public Set<Int> {
public:
  static bool isPositiveLiteral(Int literal);
  static Int getVar(Int literal);
  void printClause() const;
  Set<Int> getClauseVars() const;
};

class Graph { // undirected
public:
  Set<Int> vertices;
  Map<Int, Set<Int>> adjacencyMap;

  Graph(const Set<Int>& vs);

  bool isNeighbor(Int v1, Int v2) const;
  bool hasPath(Int from, Int to, Set<Int>& visitedVertices) const; // path length >= 0
  bool hasPath(Int from, Int to) const;
  void addEdge(Int v1, Int v2);
  void removeVertex(Int v); // also removes edges from/to v
  void fillInEdges(Int v); // does not remove v
  Int countFillInEdges(Int v) const;
  Int getMinfillVertex() const;
};

class Label : public vector<Int> { // for lexicographic search
public:
  static bool hasSmallerLabel(const pair<Int, Label>& a, const pair <Int, Label>& b);
  void addNumber(Int i); // retains descending order
};

class Cnf {
public:
  vector<Clause> clauses;
  Int declaredVarCount = 0;
  Set<Int> additiveVars; // as opposed to existential vars
  Map<Int, Number> literalWeights; // for additive and disjunctive vars
  Map<Int, Set<Int>> varToClauses; // var |-> clause indices
  Set<Int> apparentVars;

  static string helpWeightFormat();
  void printClauses() const;
  void printLiteralWeights() const;
  Set<Int> getDisjunctiveVars() const;

  void addClause(const Clause& clause);
  void setApparentVars();
  Graph getGaifmanGraph() const;
  vector<Int> getRandomVarOrder() const;
  vector<Int> getDeclaredVarOrder() const;
  vector<Int> getMostClausesVarOrder() const;
  vector<Int> getMinfillVarOrder() const;
  vector<Int> getMcsVarOrder() const;
  vector<Int> getLexpVarOrder() const;
  vector<Int> getLexmVarOrder() const;
  vector<Int> getCnfVarOrder(Int cnfVarOrderHeuristic) const;

  Cnf(); // contructs empty CNF
  Cnf(string filePath);
};

/* classes for jointrees ==================================================== */

class JoinNode { // abstract
public:
  static Int nodeCount;
  static Int terminalCount;
  static Set<Int> nonterminalIndices;

  static Int backupNodeCount;
  static Int backupTerminalCount;
  static Set<Int> backupNonterminalIndices;

  static Cnf cnf; // must be set exactly once before any object is constructed

  Int nodeIndex = MIN_INT; // 0-indexing (equal to clauseIndex for JoinTerminal)
  vector<JoinNode*> children; // empty for JoinTerminal
  Set<Int> projectionVars; // empty for JoinTerminal
  Set<Int> preProjectionVars; // set by constructor

  static void resetStaticFields(); // backs up and clears static fields
  static void restoreStaticFields(); // from backup

  virtual Int getWidth(const Assignment& assignment = Assignment()) const = 0; // of subtree

  virtual void updateVarSizes(
    Map<Int, size_t>& varSizes // var x |-> size of biggest node containing x
  ) const = 0;

  Set<Int> getPostProjectionVars() const;
  Int chooseClusterIndex(
    Int clusterIndex, // of this node
    const vector<Set<Int>>& projectableVarSets, // Z_1..Z_m
    string clusteringHeuristic
  ); // target = |projectableVarSets| if projectableVars \cap postProjectionVars = \emptyset else clusterIndex < target < |projectableVarSets|
  Int getNodeRank(
    const vector<Int>& restrictedVarOrder,
    string clusteringHeuristic
  ); // rank = |restrictedVarOrder| if restrictedVarOrder \cap postProjectionVars = \emptyset else 0 \le rank < |restrictedVarOrder|
  bool isTerminal() const;
};

class JoinTerminal : public JoinNode {
public:
  Int getWidth(const Assignment& assignment = Assignment()) const override;

  void updateVarSizes(Map<Int, size_t>& varSizes) const override;

  JoinTerminal();
};

class JoinNonterminal : public JoinNode {
public:

  void printNode(string startWord) const; // 1-indexing
  void printSubtree(string startWord = "") const; // post-order traversal

  Int getWidth(const Assignment& assignment = Assignment()) const override;

  void updateVarSizes(Map<Int, size_t>& varSizes) const override;
  vector<Int> getBiggestNodeVarOrder() const;
  vector<Int> getHighestNodeVarOrder() const;
  vector<Int> getVarOrder(Int varOrderHeuristic) const;

  vector<Assignment> getAdditiveAssignments(Int varOrderHeuristic, Int threads) const; // vars <= floor(log2(threads))

  JoinNonterminal(
    const vector<JoinNode*>& children,
    const Set<Int>& projectionVars = Set<Int>(),
    Int requestedNodeIndex = MIN_INT
  );
};
