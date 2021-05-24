#pragma once

/* inclusions =============================================================== */

#include "../libraries/cudd/cplusplus/cuddObj.hh"
#include "../libraries/cudd/cudd/cuddInt.h"

#include "../libraries/sylvan/src/sylvan_gmp.h"
#include "../libraries/sylvan/src/sylvan_obj.hpp"

#include "../libraries/cxxopts/include/cxxopts.hpp"

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

using cxxopts::value;

/* consts =================================================================== */

const Float MEGA = 1e6; // same as countAntom (1 MB = 1e6 B)

const string WEIGHTED_COUNTING_OPTION = "wc";
const string PLANNER_WAIT_OPTION = "pw";
const string THREAD_COUNT_OPTION = "tc";
const string THREAD_SLICE_COUNT_OPTION = "ts";
const string DD_VAR_OPTION = "dv";
const string SLICE_VAR_OPTION = "sv";
const string MEM_SENSITIVITY_OPTION = "ms";
const string MAX_MEM_OPTION = "mm";
const string TABLE_RATIO_OPTION = "tr";
const string INIT_RATIO_OPTION = "ir";
const string MULTIPLE_PRECISION_OPTION = "mp";
const string LOG_COUNTING_OPTION = "lc";
const string JOIN_PRIORITY_OPTION = "jp";
const string VERBOSE_JOIN_TREE_OPTION = "vj";
const string VERBOSE_PROFILING_OPTION = "vp";

const string FIRST_JOIN_TREE = "f";
const string TIMED_JOIN_TREES = "t";
const map<string, string> PLANNING_STRATEGIES = {
  {FIRST_JOIN_TREE, "FIRST_JOIN_TREE"},
  {TIMED_JOIN_TREES, "TIMED_JOIN_TREES"}
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

extern Int dotFileIndex;

extern string planningStrategy;
extern string ddPackage;
extern Int threadCount;
extern Int threadSliceCount; // may be lower or higher than actual number of slices per thread
extern Float memSensitivity; // in MB (1e6 B)
extern Float maxMem; // in MB (1e6 B)
extern string joinPriority;
extern Int verboseJoinTree; // 1: parsed join tree, 2: raw join tree too
extern Int verboseProfiling; // 1: sorted stats for cnf vars, 2: unsorted stats for join nodes too

/* classes for processing join trees ======================================== */

class JoinTree { // for JoinTreeProcessor
public:
  Int declaredVarCount = MIN_INT;
  Int declaredClauseCount = MIN_INT;
  Int declaredNodeCount = MIN_INT;

  Map<Int, JoinTerminal*> joinTerminals; // 0-indexing
  Map<Int, JoinNonterminal*> joinNonterminals; // 0-indexing

  Int width = MIN_INT; // width of latest join tree
  Float plannerDuration = -INF; // cumulative time for all join trees, in seconds

  JoinNode* getJoinNode(Int nodeIndex) const; // 0-indexing
  JoinNonterminal* getJoinRoot() const;
  void printTree() const;

  JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount);
};

class JoinTreeProcessor { // processes input join tree
public:
  static Int plannerPid;

  JoinTree* joinTree = nullptr;
  Int lineIndex = 0;
  Int problemLineIndex = MIN_INT;

  const JoinNonterminal* getJoinTreeRoot() const;
  static void killPlanner(); // sends SIGKILL
};

class JoinTreeParser : public JoinTreeProcessor { // first join tree
public:
  void finishParsingJoinTree(); // after "c seconds {float1}" or end of stream
  void parseInputStream();

  JoinTreeParser();
};

class JoinTreeReader : public JoinTreeProcessor { // timed join trees
public:
  JoinTree* backupJoinTree = nullptr;
  Int joinTreeEndLineIndex = MIN_INT;

  /* timer: */
  static void handleSigalrm(int signal); // kills planner after receiving SIGALRM
  static bool hasDisarmedTimer();
  static void setTimer(Float seconds); // arms or disarms timer
  static void armTimer(Float seconds); // schedules SIGALRM
  static void disarmTimer(); // in case stdin ends before timer expires

  void finishReadingJoinTree(); // after "c seconds {float1}" or end of stream
  void readInputStream();

  JoinTreeReader(Float plannerWaitDuration);
};

/* classes for decision diagrams ============================================ */

class Dd { // wrapper for CUDD and Sylvan
public:
  ADD cuadd; // CUDD
  Mtbdd mtbdd; // Sylvan

  Dd(const ADD& cuadd); // CUDD
  Dd(const Mtbdd& mtbdd); // SYLVAN
  Dd(const Dd& dd);

  static const Cudd* newMgr(Float mem, Int threadIndex); // CUDD
  static Dd getConstDd(const Number& n, const Cudd* mgr);
  static Dd getZeroDd(const Cudd* mgr);
  static Dd getOneDd(const Cudd* mgr);
  static Dd getVarDd(Int ddVar, bool val, const Cudd* mgr);
  size_t countNodes() const;
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
  static void writeInfoFile(const Cudd* mgr, string filePath);
};

class Executor {
public:
  static Map<Int, Float> varDurations; // cnfVar |-> total execution time in seconds
  static Map<Int, size_t> varDdSizes; // cnfVar |-> max ADD size

  static void updateVarDurations(const JoinNode* joinNode, TimePoint startPoint);
  static void updateVarDdSizes(const JoinNode* joinNode, const Dd& dd);

  static void printVarDurations();
  static void printVarDdSizes();

  static Dd getClauseDd(
    const Map<Int, Int>& cnfVarToDdVarMap,
    const Clause& clause,
    const Cudd* mgr,
    const Assignment& assignment
  );
  static Dd solveSubtree(
    const JoinNode* joinNode,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr = nullptr,
    const Assignment& assignment = Assignment()
  );
  static void solveThreadSlices( // sequentially solves all slices in 1 thread
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    Float threadMem,
    Int threadIndex,
    const vector<vector<Assignment>>& threadAssignmentLists,
    Number& totalSolution,
    mutex& solutionMutex
  );
  static vector<vector<Assignment>> getThreadAssignmentLists(
    const JoinNonterminal* joinRoot,
    Int sliceVarOrderHeuristic
  );
  static Number solveCnf(
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    Int sliceVarOrderHeuristic
  );

  static Number adjustSolution(const Number &apparentSolution); // takes into account hidden vars

  static void printSatRow(const Number& solution, bool surelyUnsat, size_t keyWidth); // "s {satisfiability}"
  static void printTypeRow(size_t keyWidth); // "c s type {track}"
  static void printEstRow(const Number& solution, size_t keyWidth); // "c s log10-estimate {log(count)}"
  static void printArbRow(const Number& solution, bool frac, size_t keyWidth); // "c s exact arb {notation} {count}"
  static void printDoubleRow(const Number& solution, size_t keyWidth); // "c s exact double prec-sci {count}"
  static void printSolutionRows(const Number& solution, bool surelyUnsat = false, size_t keyWidth = 0);

  Executor(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, Int sliceVarOrderHeuristic);
};

class OptionDict {
public:
  string cnfFilePath;
  Float plannerWaitDuration;
  Int ddVarOrderHeuristic;
  Int sliceVarOrderHeuristic;
  Int tableRatio; // log2(unique_table / cache_table)
  Int initRatio; // log2(max_size / init_size)

  static string helpDdPackage();
  static string helpJoinPriority();

  void runCommand() const;

  OptionDict(int argc, char** argv);
};

/* global functions ========================================================= */

int main(int argc, char** argv);
