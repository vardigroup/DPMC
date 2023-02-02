#pragma once

/* inclusions =============================================================== */

#include "../libraries/cxxopts/include/cxxopts.hpp"

#include "../libraries/cudd/cplusplus/cuddObj.hh"
#include "../libraries/cudd/cudd/cuddInt.h"

#include "../libraries/sylvan/src/sylvan_gmp.h"
#include "../libraries/sylvan/src/sylvan_obj.hpp"

#include "../libraries/cryptominisat/build/include/cryptominisat5/cryptominisat.h"

#include "common.hh"

/* uses ===================================================================== */

using sylvan::gmp_op_max_CALL;
using sylvan::gmp_op_plus_CALL;
using sylvan::gmp_op_times_CALL;
using sylvan::mtbdd_apply_CALL;
using sylvan::mtbdd_fprintdot_nc;
using sylvan::mtbdd_getdouble;
using sylvan::mtbdd_getvalue;
using sylvan::mtbdd_gmp;
using sylvan::mtbdd_leafcount_more;
using sylvan::mtbdd_makenode;
using sylvan::Mtbdd;
using sylvan::MTBDD;

using sylvan::Bdd;

using CMSat::Lit;
using CMSat::lbool; // generally uint8_t; typically {l_True, l_False, l_Undef}
using CMSat::l_True;
using CMSat::l_False;

using util::printRow;

/* consts =================================================================== */

const Float MEGA = 1e6l; // same as countAntom (1 MB = 1e6 B)

const string WEIGHTED_COUNTING_OPTION = "wc";
const string EXIST_RANDOM_OPTION = "er";
const string LOG_COUNTING_OPTION = "lc";
const string LOG_BOUND_OPTION = "lb";
const string THRESHOLD_MODEL_OPTION = "tm";
const string SAT_SOLVER_PRUNING = "sp";
const string MAXIMIZER_FORMAT_OPTION = "mf";
const string MAXIMIZER_VERIFICATION_OPTION = "mv";
const string SUBSTITUTION_MAXIMIZATION_OPTION = "sm";
const string PLANNER_WAIT_OPTION = "pw";
const string THREAD_COUNT_OPTION = "tc";
const string THREAD_SLICE_COUNT_OPTION = "ts";
const string DD_VAR_OPTION = "dv";
const string DYN_ORDER_OPTION = "dy";
const string SAT_FILTER_OPTION = "sa";
const string ATOMIC_ABSTRACT_OPTION = "aa";
const string SLICE_VAR_OPTION = "sv";
const string MEM_SENSITIVITY_OPTION = "ms";
const string MAX_MEM_OPTION = "mm";
const string TABLE_RATIO_OPTION = "tr";
const string INIT_RATIO_OPTION = "ir";
const string MULTIPLE_PRECISION_OPTION = "mp";
const string JOIN_PRIORITY_OPTION = "jp";
const string VERBOSE_JOIN_TREE_OPTION = "vj";
const string VERBOSE_PROFILING_OPTION = "vp";

/* maximizer formats: */
const Int NEITHER_FORMAT = 0;
const Int SHORT_FORMAT = 1;
const Int LONG_FORMAT = 2;
const Int DUAL_FORMAT = 3;
const map<Int, string> MAXIMIZER_FORMATS = {
  {NEITHER_FORMAT, "NEITHER"},
  {SHORT_FORMAT, "SHORT"},
  {LONG_FORMAT, "LONG"},
  {DUAL_FORMAT, "DUAL"}
};

/* join priorities: */
const string ARBITRARY_PAIR = "a";
const string BIGGEST_PAIR = "b";
const string SMALLEST_PAIR = "s";
const string FCFS = "f";
const map<string, string> JOIN_PRIORITIES = {
  {ARBITRARY_PAIR, "ARBITRARY_PAIR"},
  {BIGGEST_PAIR, "BIGGEST_PAIR"},
  {SMALLEST_PAIR, "SMALLEST_PAIR"},
  {FCFS, "FCFS"}
};

/* global vars ============================================================== */

extern bool existRandom;
extern string ddPackage;
extern bool logCounting;
extern Float logBound;
extern string thresholdModel;
extern bool satSolverPruning;
extern Int maximizerFormat;
extern bool maximizerVerification;
extern bool substitutionMaximization;
extern Int threadCount;
extern Int threadSliceCount; // may be lower or higher than actual number of slices per thread
extern Float memSensitivity; // in MB (1e6 B)
extern Float maxMem; // in MB (1e6 B)
extern string joinPriority;
extern Int verboseJoinTree; // 1: parsed join tree, 2: raw join tree too
extern Int verboseProfiling; // 1: sorted stats for CNF vars, 2: unsorted stats for join nodes too

extern Int dotFileIndex;

extern Int dynVarOrdering;
extern bool atomicAbstract;

/* classes for processing join trees ======================================== */

class JoinTree { // for JoinTreeProcessor
public:
  Int declaredVarCount = MIN_INT;
  Int declaredClauseCount = MIN_INT;
  Int declaredNodeCount = MIN_INT;

  Map<Int, JoinTerminal*> joinTerminals; // 0-indexing
  Map<Int, JoinNonterminal*> joinNonterminals; // 0-indexing

  Int width = MIN_INT; // width of latest join tree
  Float plannerDuration = 0; // cumulative time for all join trees, in seconds

  JoinNode* getJoinNode(Int nodeIndex) const; // 0-indexing
  JoinNonterminal* getJoinRoot() const;
  void printTree() const;

  JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount);
};

class JoinTreeProcessor {
public:
  static Int plannerPid;
  static JoinTree* joinTree;
  static JoinTree* backupJoinTree;

  Int lineIndex = 0;
  Int problemLineIndex = MIN_INT;
  Int joinTreeEndLineIndex = MIN_INT;

  static void killPlanner(); // sends SIGKILL

  /* timer: */
  static void handleSigAlrm(int signal); // kills planner after receiving SIGALRM
  static bool hasDisarmedTimer();
  static void setTimer(Float seconds); // arms or disarms timer
  static void armTimer(Float seconds); // schedules SIGALRM
  static void disarmTimer(); // in case stdin ends before timer expires

  const JoinNonterminal* getJoinTreeRoot() const;

  void processCommentLine(const vector<string>& words);
  void processProblemLine(const vector<string>& words);
  void processNonterminalLine(const vector<string>& words);

  void finishReadingJoinTree();
  void readInputStream();

  JoinTreeProcessor(Float plannerWaitDuration);
};

/* classes for execution ==================================================== */

class SatSolver {
public:
  CMSat::SATSolver cmsSolver;

  bool checkSat(bool exceptionThrowing); // may throw UnsatSolverException
  Assignment getModel(); // also bans returned model for future solving
  Lit getLit(Int cnfVar, bool val);
  SatSolver(const Cnf& cnf);
};

class Dd { // wrapper for CUDD and Sylvan
public:
  static size_t maxDdLeafCount;
  static size_t maxDdNodeCount;

  static size_t prunedDdCount;
  static Float pruningDuration;
 
  ADD cuadd; // CUDD
  Mtbdd mtbdd; // Sylvan

  static bool dynOrderEnabled;

  size_t getLeafCount() const;
  size_t getNodeCount() const;

  Dd(const ADD& cuadd);
  Dd(const Mtbdd& mtbdd);
  Dd(const Dd& dd);

  Number extractConst() const; // does not read logCounting
  static Dd getConstDd(const Number& n, const Cudd* mgr); // reads logCounting
  static Dd getZeroDd(const Cudd* mgr); // returns minus infinity if logCounting
  static Dd getOneDd(const Cudd* mgr); // returns zero if logCounting
  static Dd getVarDd(Int ddVar, bool val, const Cudd* mgr);
  static const Cudd* newMgr(Float mem, Int threadIndex = 0); // CUDD
  bool operator!=(const Dd& rightDd) const;
  bool operator<(const Dd& rightDd) const; // *this < rightDd (top of priotity queue is rightmost element)
  Dd getComposition(Int ddVar, bool val, const Cudd* mgr) const; // restricts *this to ddVar=val
  Dd getProduct(const Dd& dd) const; // reads logCounting
  Dd getSum(const Dd& dd) const; // reads logCounting
  Dd getMax(const Dd& dd) const; // real max (not 0-1 max)
  Dd getXor(const Dd& dd) const; // must be 0-1 DDs
  Set<Int> getSupport() const;
  Dd getBoolDiff(const Dd& rightDd) const; // returns 0-1 DD for *this >= rightDd
  bool evalAssignment(vector<int>& ddVarAssignment) const;
  Dd getAbstraction(
    Int ddVar,
    const vector<Int>& ddVarToCnfVarMap,
    const Map<Int, Number>& literalWeights,
    const Assignment& assignment,
    bool additiveFlag, // ? getSum : getMax
    vector<pair<Int, Dd>>& maximizationStack,
    const Cudd* mgr
  ) const;
  Dd getPrunedDd(Float lowerBound, const Cudd* mgr) const;
  void writeDotFile(const Cudd* mgr, const string& dotFileDir = "./") const;
  static void writeInfoFile(const Cudd* mgr, const string& filePath);
  
  static bool enableDynamicOrdering(const Cudd* mgr);
  static bool disableDynamicOrdering(const Cudd* mgr);
  static int postReorderHook(DdManager *dd, const char *str, void *data);
  static int preReorderHook(DdManager *dd, const char *str, void *data);

  BDD cubdd; 
  Bdd sybdd;

  static Dd getZeroBdd(const Cudd* mgr); // returns minus infinity if logCounting
  static Dd getOneBdd(const Cudd* mgr); // returns zero if logCounting
  static Dd getVarBdd(Int ddVar, bool val, const Cudd* mgr);
  Dd(const BDD& cubdd);
  Dd(const Bdd& sybdd);

  Dd getBddExists(
    vector<Int> ddVars,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr
  ) const;
  Dd getBddAnd(const Dd& dd) const;
  Dd getBddOr(const Dd& dd) const; // must be Bdd
  bool isTrue() const; // must be Bdd
  Set<Int> getBddSupport() const;
  Dd getFilteredBdd(const Dd, const Cudd* mgr);
  Dd getAdd();
  Dd getAddSumAbstract(const Set<Int>& cnfVars, const Map<Int,Int>& cnfVarToDdVarMap, Map<Int, Number>& literalWeights, const Cudd* mgr);
};

class Executor {
public:
  static TimePoint executorStartPoint;
  static Int joinNodesProcessed;

  static vector<Int> d2cMap;

  static vector<pair<Int, Dd>> maximizationStack; // pair<DD var, derivative sign>

  static Map<Int, Float> varDurations; // CNF var |-> total execution time in seconds
  static Map<Int, size_t> varDdSizes; // CNF var |-> max DD size

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
  static Dd solveSubtree( // recursively computes valuation of join tree node
    const JoinNode* joinNode,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr = nullptr,
    const Assignment& assignment = Assignment()
  );
  static void solveThreadSlices( // sequentially solves all slices in one thread
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

  static void setLogBound(
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap
  );

  static Number adjustSolutionToHiddenVar(const Number &apparentSolution, Int cnfVar, bool additiveFlag);
  static Number getAdjustedSolution(const Number &apparentSolution);

  static void printSatRow(const Number& solution, bool unsatFlag, size_t keyWidth); // "s {satisfiability}"
  static void printTypeRow(size_t keyWidth); // "c s type {track}"
  static void printEstRow(const Number& solution, size_t keyWidth); // "c s log10-estimate {log(sol)}"
  static void printArbRow(const Number& solution, bool frac, size_t keyWidth); // "c s exact arb {notation} {sol}"
  static void printDoubleRow(const Number& solution, size_t keyWidth); // "c s exact double prec-sci {sol}"
  static Number printAdjustedSolutionRows(const Number& solution, bool unsatFlag = false, size_t keyWidth = 0); // returns adjusted solution

  static string getShortModel(const Assignment& model, Int declaredVarCount);
  static string getLongModel(const Assignment& model, Int declaredVarCount);
  static void printShortMaximizer(const Assignment& maximizer, Int declaredVarCount);
  static void printLongMaximizer(const Assignment& maximizer, Int declaredVarCount);
  static Assignment printMaximizerRows(const vector<Int>& ddVarToCnfVarMap, Int declaredVarCount); // returns maximizer
  static Number verifyMaximizer( // returns solution of residual formula
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Assignment& maximizer
  );

  Executor(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, Int sliceVarOrderHeuristic, const Cudd* mgr, const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap);
};

class SatFilter{
  public:
  //static vector<pair<Int, Dd>> maximizationStack; // pair<DD var, derivative sign> Retained here just as a dummy placeholder in solveSubtree
  /*
  static Map<Int, Float> varDurations; // CNF var |-> total execution time in seconds
  static Map<Int, size_t> varDdSizes; // CNF var |-> max DD size

  static void updateVarDurations(const JoinNode* joinNode, TimePoint startPoint);
  static void updateVarDdSizes(const JoinNode* joinNode, const Dd& dd);

  static void printVarDurations();
  static void printVarDdSizes();
  */
  static TimePoint constructFilterStartPoint;
  static TimePoint satFilterStartPoint;
  static Int joinNodesProcessed;

  static Dd getClauseBdd(
    const Map<Int, Int>& cnfVarToDdVarMap,
    const Clause& clause,
    const Cudd* mgr
  );
  static Dd solveSubtree( // recursively computes valuation of join tree node
    const JoinNode* joinNode,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr = nullptr
  );

  static bool solveCnf(
    const JoinNonterminal* joinRoot,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Cudd* mgr
  );

  static bool filterBdds(
    const JoinNode* joinNode,
    const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap,
    const Dd parentBdd,
    const Cudd* mgr
  );

  SatFilter(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, const Cudd* mgr, const Map<Int, Int>& cnfVarToDdVarMap,
    const vector<Int>& ddVarToCnfVarMap);
};

class OptionRequirement {
public:
  string name;
  string value;
  string comparator;

  OptionRequirement(const string& name, const string& value, const string& comparator = "=");
  string getRequirement() const;
};

class OptionDict {
public:
  string cnfFilePath;
  Float plannerWaitDuration;
  Int ddVarOrderHeuristic;
  Int sliceVarOrderHeuristic;
  Int tableRatio; // log2(unique_table / cache_table)
  Int initRatio; // log2(max_size / init_size)

  static string requireOptions(const vector<OptionRequirement>& requirements);
  static string requireOption(const string& name, const string& value, const string& comparator = "=");
  static string requireDdPackage(const string& ddPackageArg);

  static string helpDdPackage();
  static string helpLogBound();
  static string helpThresholdModel();
  static string helpSatSolverPruning();
  static string helpMaximizerFormat();
  static string helpSubstitutionMaximization();
  static string helpDiagramVarOrderHeuristic();
  static string helpSliceVarOrderHeuristic();
  static string helpJoinPriority();
  static string helpDynamicVarOrdering();
  static string helpSatFilter();
  static string helpAtomicAbstract();

  void runCommand() const;

  OptionDict(int argc, char** argv);
};

/* global functions ========================================================= */

int main(int argc, char** argv);
