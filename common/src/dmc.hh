/* inclusions =============================================================== */

#include "../lib/cudd/cplusplus/cuddObj.hh"
#include "../lib/cudd/cudd/cuddInt.h"

#include "../lib/sylvan/src/sylvan_gmp.h"
#include "../lib/sylvan/src/sylvan_obj.hpp"

#include "../lib/cxxopts/include/cxxopts.hpp"

#include "logic.hh"

/* uses ===================================================================== */

using sylvan::gmp_op_max_CALL;
using sylvan::gmp_op_plus_CALL;
using sylvan::gmp_op_times_CALL;
using sylvan::mtbdd_apply_CALL;
using sylvan::mtbdd_fprintdot_nc;
using sylvan::mtbdd_getdouble;
using sylvan::mtbdd_getvalue;
using sylvan::mtbdd_gmp;
using sylvan::mtbdd_makenode;
using sylvan::Mtbdd;
using sylvan::MTBDD;

using cxxopts::value;

/* consts =================================================================== */

const string PLANNING_STRATEGY_OPTION = "ps";
const string JT_WAIT_OPTION = "jw";
const string DD_VAR_OPTION = "dv";
const string SLICE_VAR_OPTION = "sv";
const string MAX_MEM_OPTION = "mm";
const string INIT_RATIO_OPTION = "ir";
const string TABLE_RATIO_OPTION = "tr";
const string MULTIPLE_PRECISION_OPTION = "mp";
const string LOG_COUNTING_OPTION = "lc";
const string JOIN_PRIORITY_OPTION = "jp";

const string FIRST_JOINTREE = "f";
const string TIMED_JOINTREES = "t";
const map<string, string> PLANNING_STRATEGIES = {
  {FIRST_JOINTREE, "FIRST_JOINTREE"},
  {TIMED_JOINTREES, "TIMED_JOINTREES"}
};

const string ARBITRARY_PAIR = "a";
const string BIGGEST_PAIR = "b";
const string SMALLEST_PAIR = "s";
const map<string, string> JOIN_PRIORITIES = {
  {ARBITRARY_PAIR, "ARBITRARY_PAIR"},
  {BIGGEST_PAIR, "BIGGEST_PAIR"},
  {SMALLEST_PAIR, "SMALLEST_PAIR"}
};

/* global vars ============================================================== */

extern string planningStrategy;
extern Int threadCount;
extern string ddPackage;
extern Int maxMegabytes;
extern string joinPriority;

/* classes for processing jointrees ========================================= */

class JoinTree { // for JoinTreeProcessor
public:
  Int declaredVarCount = MIN_INT;
  Int declaredClauseCount = MIN_INT;
  Int declaredNodeCount = MIN_INT;

  Map<Int, JoinTerminal*> joinTerminals; // 0-indexing
  Map<Int, JoinNonterminal*> joinNonterminals; // 0-indexing

  Float plannerSeconds = -INF; // cumulative time for all jointrees

  JoinNode* getJoinNode(Int nodeIndex) const; // 0-indexing
  JoinNonterminal* getJoinRoot() const;
  void printTree() const;

  JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount);
};

class JoinTreeProcessor { // processes input jointree
public:
  static Int plannerPid;

  JoinTree* joinTree = nullptr;
  Int lineIndex = 0;
  Int problemLineIndex = MIN_INT;

  static void killPlanner(); // sends SIGKILL
  const JoinNonterminal* getJoinTreeRoot() const;

  JoinTreeProcessor();
};

class JoinTreeParser : public JoinTreeProcessor { // first jointree
public:
  void finishParsingJoinTree(); // after "c seconds $secs" or end of stream
  void parseInputStream();

  JoinTreeParser();
};

class JoinTreeReader : public JoinTreeProcessor { // timed jointrees
public:
  JoinTree* backupJoinTree = nullptr;
  Int joinTreeEndLineIndex = MIN_INT;

  /* timer: */
  static void handleSigalrm(int signal); // kills planner after receiving SIGALRM
  static bool hasDisarmedTimer();
  static void setTimer(Float seconds); // arms or disarms timer
  static void disarmTimer(); // in case STDIN ends before timer expires
  static void armTimer(Float seconds); // schedules SIGALRM after `seconds`

  void finishReadingJoinTree(); // after "c seconds $secs" or end of stream
  void readInputStream();

  JoinTreeReader(Float jtWaitSeconds);
};

/* classes for decision diagrams ============================================ */

class Dd { // wrapper for CUDD and Sylvan
public:
  ADD cuadd; // CUDD
  Mtbdd mtbdd; // Sylvan

  Dd(const ADD& cuadd); // CUDD
  Dd(const Mtbdd& mtbdd); // SYLVAN
  Dd(const Dd& dd);

  static const Cudd* newMgr(Int maxMegs); // CUDD
  static Dd getConstDd(const Number& n, const Cudd* mgr);
  static Dd getZeroDd(const Cudd* mgr);
  static Dd getOneDd(const Cudd* mgr);
  static Dd getVarDd(Int ddVar, bool val, const Cudd* mgr);
  Int countNodes() const;
  bool operator<(const Dd& rightDd) const; // *this < rightDd (top of priotity queue is rightmost element)
  Number extractConst() const;
  Dd getComposition(Int ddVar, bool val, const Cudd* mgr) const; // restricts *this to ddVar=val
  Dd getProduct(const Dd& dd) const;
  Dd getSum(const Dd& dd) const;
  Dd getMax(const Dd& dd) const; // real max (not 0-1 max)
  Set<Int> getSupport() const;
  Dd getAbstraction(
    Int ddVar,
    const vector<Int>& ddVarToCnfVarMap,
    const Map<Int, Number>& literalWeights,
    const Assignment& assignment,
    bool additive, // ? getSum : getMax
    const Cudd* mgr
  ) const;
  void writeDotFile(const Cudd* mgr, string dotFileDir = "./") const;
};

class Executor {
public:
  static Dd getClauseDd(
    const Map<Int, Int>& cnfVarToDdVarMap,
    const Clause& clause,
    const Cudd* mgr,
    const Assignment& assignment
  );
  static Dd countSubtree(
    const JoinNode* joinNode,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr,
    const Assignment& assignment = Assignment()
  );
  static void countSlicedCnf(
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr,
    const Assignment& assignment,
    Int threadIndex,
    Float& totalCount,
    mutex& countMutex
  );
  static Number countCnf(
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    Int sliceVarOrderHeuristic
  );
  static void printSolutionLine(const Number& modelCount);

  Executor(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, Int sliceVarOrderHeuristic);
};

class OptionDict {
public:
  string cnfFilePath;
  Float jtWaitSeconds;
  Int ddVarOrderHeuristic;
  Int sliceVarOrderHeuristic;
  Int initRatio; // log2(max_mem / init_size)
  Int tableRatio; // log2(unique_table / cache_table)

  cxxopts::Options* options;

  static string helpPlanningStrategy();
  static string helpDdPackage();
  static string helpJoinPriority();

  void solveOptions() const;

  OptionDict(int argc, char** argv);
};

/* global functions ========================================================= */

int main(int argc, char** argv);
