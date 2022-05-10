#pragma once

/* inclusions *****************************************************************/

#include "formula.hpp"

/* constants ******************************************************************/

extern const string &JT_WORD;
extern const string &VAR_ELIM_WORD;

/* classes ********************************************************************/

class JoinNode { // abstract
protected:
  static Int backupNodeCount;
  static Int backupTerminalCount;
  static Set<Int> backupNonterminalIndexes;

  static Int nodeCount;
  static Int terminalCount;
  static Set<Int> nonterminalIndexes;

  Int nodeIndex = DUMMY_MIN_INT; // 0-indexing (equal to clauseIndex for JoinTerminal)
  vector<JoinNode *> children; // empty for JoinTerminal
  Set<Int> projectableVars; // empty for JoinTerminal

  Set<Int> preProjectionVars; // apparent vars after projecting every child->projectableVars and before projecting this->projectableVars
  bool evaluatedPreProjectionVars = false; // for lazy evaluation

public:
  static void resetStaticFields(); // backs up and clears static fields
  static void restoreStaticFields(); // from backup

  static Int getNodeCount();
  static Int getTerminalCount();

  bool isTerminal() const;
  Int getNodeIndex() const;
  const vector<JoinNode *> &getChildren() const;
  const Set<Int> &getProjectableVars() const;

  Int chooseClusterIndex(
    Int clusterIndex, // of this node
    vector<Set<Int>> projectableVarSets, // Z_1..Z_m
    ClusteringHeuristic clusteringHeuristic,
    const vector<vector<Int>> &clauses
  ); // target = |projectableVarSets| if projectableVars \cap postProjectionVars = \emptyset else clusterIndex < target < |projectableVarSets|
  Int getNodeRank(const vector<Int> &restrictedVarOrdering, ClusteringHeuristic clusteringHeuristic, const vector<vector<Int>> &clauses); // rank = |restrictedVarOrdering| if restrictedVarOrdering \cap postProjectionVars = \emptyset else 0 \le rank < |restrictedVarOrdering|
  Set<Int> getPostProjectionVars(const vector<vector<Int>> &clauses);

  virtual const Set<Int> &getPreProjectionVars(const vector<vector<Int>> &clauses) = 0; // lazy evaluation
  virtual Int getMaxVarCount(const vector<vector<Int>> &clauses) = 0;
  virtual void printSubtree(const string &prefix = "") const = 0;
};

class JoinTerminal : public JoinNode {
public:
  const Set<Int> &getPreProjectionVars(const vector<vector<Int>> &clauses) override;
  Int getMaxVarCount(const vector<vector<Int>> &clauses) override; // width of project-join tree
  void printSubtree(const string &prefix = "") const override;
  JoinTerminal();
};

class JoinNonterminal : public JoinNode {
public:
  void printNode(const string &prefix) const; // 1-indexing
  const Set<Int> &getPreProjectionVars(const vector<vector<Int>> &clauses) override;
  Int getMaxVarCount(const vector<vector<Int>> &clauses) override;
  void printSubtree(const string &prefix = "") const override; // post-order
  void addProjectableVars(const Set<Int> &vars);
  JoinNonterminal(
    const vector<JoinNode *> &children,
    const Set<Int> &projectableVars = Set<Int>(),
    Int requestedNodeIndex = DUMMY_MIN_INT
  );
};

class JoinComponent { // for projected model counting
  Cnf cnf;
  VarOrderingHeuristic varOrderingHeuristic;
  bool inverseVarOrdering;
  ClusteringHeuristic clusteringHeuristic;
  vector<JoinNode *> subtrees; // R
  Set<Int> keptVars; // F

  Set<Int> projectableVars; // Z
  vector<Set<Int>> projectableVarSets; // Z_1..Z_m is a partition of Z
  vector<vector<JoinNode *>> nodeClusters; // kappa_0..kappa_m: kappa_0 is nodeClusters.back()

  Set<Int> getNodeVars(vector<JoinNode *> nodes) const;
  void computeFields();

public:
  JoinNonterminal *getComponentRoot();
  JoinComponent(const Cnf &cnf, VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering, ClusteringHeuristic clusteringHeuristic, const vector<JoinNode *> &subtrees, const Set<Int> &keptVars);
};

class JoinRootBuilder {
  Cnf cnf;

  Set<Int> disjunctiveVars;
  vector<Set<Int>> disjunctiveVarSets; // clause index |-> vars

  vector<vector<Int>> clauseGroups; // group |-> clause indexes

  void printDisjunctiveVarSets() const;
  void printClauseGroups() const;

  void setDisjunctiveVarSets(); // also sets disjunctiveVars
  void setClauseGroups();

public:
  JoinNonterminal *buildRoot(VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering, ClusteringHeuristic clusteringHeuristic);
  JoinRootBuilder(const Cnf &cnf);
};

class JoinTree { // for JoinTreeProcessor
  friend class JoinTreeParser;
  friend class JoinTreeReader;

protected:
  Int declaredVarCount = DUMMY_MIN_INT; // in jt file
  Int declaredClauseCount = DUMMY_MIN_INT; // in jt file
  Int declaredNodeCount = DUMMY_MIN_INT; // in jt file

  Map<Int, JoinTerminal *> joinTerminals;
  Map<Int, JoinNonterminal *> joinNonterminals;

  Float plannerSeconds = NEGATIVE_INFINITY; // cumulative time for all join trees

  JoinNode *getJoinNode(Int nodeIndex) const;

public:
  JoinNonterminal *getJoinRoot() const;
  void printTree() const;

  JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount);
};

class JoinTreeProcessor { // processes input join tree
protected:
  static Int plannerPid;
  JoinTree *joinTree = nullptr;
  Int lineIndex = 0;
  Int problemLineIndex = DUMMY_MIN_INT;
  Int joinTreeEndLineIndex = DUMMY_MIN_INT;
  vector<vector<Int>> clauses;

public:
  JoinNonterminal *getJoinTreeRoot() const;
  JoinTreeProcessor(); // installs util::handleSignal for SIGINT and SIGTERM
};

class JoinTreeParser : public JoinTreeProcessor { // PlanningStrategy::FIRST_JOIN_TREE
protected:
  static void killPlanner(); // SIGKILL

  void finishParsingJoinTree(); // after reading '=' or end of stream
  void parseInputStream(std::istream *inputStream);

public:
  JoinTreeParser(
    const string &filePath,
    const vector<vector<Int>> &clauses
  );
};

class JoinTreeReader : public JoinTreeProcessor { // PlanningStrategy::TIMING
protected:
  /* timer: */
  static Float timeoutSeconds; // current time limit
  static TimePoint startPoint; // of current timer
  static Float performanceFactor;

  Float jtWaitSeconds;
  JoinTree *backupJoinTree = nullptr;

  /* timer: */
  static void killPlanner(); // SIGKILL
  static void handleAlarm(int signal); // kills planner after receiving SIGALRM
  static void setAlarm(Float seconds); // arms or disarms alarm
  static bool isDisarmed();
  static void disarmAlarm();
  static void reviseAlarm(Float newTimeoutSeconds); // reschedules SIGALRM if performanceFactor > 0 and newTimeoutSeconds < remainingSeconds
  static void initializeAlarm(Float seconds); // schedules SIGALRM after `seconds`

  Float getNewTimeoutSeconds(Int maxVarCount) const;
  void finishReadingJoinTree(bool alarming); // after reading '=' or end of stream
  void readInputStream(std::istream *inputStream);

public:
  JoinTreeReader(
    const string &filePath,
    Float jtWaitSeconds,
    Float performanceFactor,
    const vector<vector<Int>> &clauses
  );
};
