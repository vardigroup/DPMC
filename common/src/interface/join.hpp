#pragma once

/* inclusions *****************************************************************/

#include "util.hpp"
#include "sampler/utils.hpp"
/* uses ***********************************************************************/

using util::getSeconds;
using util::printComment;
using util::printThickLine;
using util::printThinLine;
using util::showError;
using util::showWarning;

/* constants ******************************************************************/

extern const string &JOIN_TREE_END_WORD;
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

  Int nodeIndex = DUMMY_MIN_INT; // 0-indexing (equal to clauseIndex for terminals)
  vector<JoinNode *> children; // empty for terminals
  Set<Int> projectableCnfVars; // empty for terminals

  Set<Int> apparentCnfVars; // lazy evaluation
  bool evaluatedApparentCnfVars = false;
  
  SamplerUtils::ADDWrapper nodeDD;

public:
  static void resetStaticFields();
  static void restoreStaticFields();

  static Int getNodeCount();
  static Int getTerminalCount();

  bool isTerminal() const;
  Int getNodeIndex() const;
  const vector<JoinNode *> &getChildren() const;
  const Set<Int> &getProjectableCnfVars() const;
  virtual const Set<Int> &getApparentCnfVars(const vector<vector<Int>> &clauses) = 0; // after projecting all child->projectableCnfVars and before projecting this->projectableCnfVars
  virtual Int getMaxVarCount(const vector<vector<Int>> &clauses) = 0;
  virtual void printSubtree(const string &prefix = "") const = 0;

  SamplerUtils::ADDWrapper& getNodeDD();
  void setNodeDD(SamplerUtils::ADDWrapper);
};

class JoinTerminal : public JoinNode {
public:
  const Set<Int> &getApparentCnfVars(const vector<vector<Int>> &clauses) override;
  virtual Int getMaxVarCount(const vector<vector<Int>> &clauses);
  void printSubtree(const string &prefix = "") const override;
  JoinTerminal();
};

class JoinNonterminal : public JoinNode {
public:
  void printNode(const string &prefix) const;
  const Set<Int> &getApparentCnfVars(const vector<vector<Int>> &clauses) override;
  virtual Int getMaxVarCount(const vector<vector<Int>> &clauses);
  void printSubtree(const string &prefix = "") const override; // post-order
  void addProjectableCnfVars(const Set<Int> &cnfVars);
  JoinNonterminal(
    const vector<JoinNode *> &children,
    const Set<Int> &projectableCnfVars = Set<Int>(),
    Int requestedNodeIndex = DUMMY_MIN_INT
  );
};

class JoinTree {
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

class JoinTreeReader {
protected:
  /* timer: */
  static Float timeoutSeconds; // current time limit
  static TimePoint startPoint; // of current timer
  static Int plannerPid;

  Float jtWaitSeconds;
  Float performanceFactor;
  JoinTree *backupJoinTree = nullptr;
  JoinTree *joinTree = nullptr;
  Int lineIndex = 0;
  Int problemLineIndex = DUMMY_MIN_INT;
  Int joinTreeEndLineIndex = DUMMY_MIN_INT;
  vector<vector<Int>> clauses;

  /* timer: */
  static void killPlanner(); // SIGKILL
  static void handleAlarm(int signal); // kills planner after receiving SIGALRM
  static void setAlarm(Float seconds); // arms or disarms alarm
  static bool isDisarmed();
  static void disarmAlarm();
  static void reviseAlarm(Float newTimeoutSeconds); // reschedules SIGALRM if newTimeoutSeconds < remainingSeconds
  static void initializeAlarm(Float seconds); // schedules SIGALRM after `seconds`

  Float getNewTimeoutSeconds(Int maxVarCount) const;
  void finishReadingJoinTree(bool alarming); // after reading '=' or end of stream
  void readInputStream(std::istream *inputStream);

public:
  JoinNonterminal *getJoinTreeRoot() const;
  JoinTreeReader(
    const string &filePath,
    Float jtWaitSeconds,
    Float performanceFactor,
    const vector<vector<Int>> &clauses
  );
};
