/* inclusions: util < graph < formula < join < counter < main* */

#pragma once

/* inclusions *****************************************************************/

#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <random>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include <boost/pending/disjoint_sets.hpp>

#include <gmp.h>
#include <gmpxx.h>
// #include "../../lib/gmp/gmp.h"
// #include "../../lib/gmp/gmpxx.h"

#include "../../lib/sylvan/src/sylvan_obj.hpp"
#include "../../lib/sylvan/src/sylvan.h"

#include "../../lib/cudd/cplusplus/cuddObj.hh"
#include "../../lib/cudd/cudd/cuddInt.h"

#include "../../lib/cxxopts/include/cxxopts.hpp"

/* uses ***********************************************************************/

using std::cerr;
using std::cout;
using std::string;
using std::to_string;
using std::vector;

using sylvan::mtbdd_fprintdot_nc;
using sylvan::mtbdd_getdouble;
using sylvan::mtbdd_makenode;
using sylvan::Mtbdd;
using sylvan::MTBDD;
using sylvan::MtbddMap;

/* types **********************************************************************/

using Float = long double; // std::stold
using Int = int_fast64_t; // std::stoll
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template<typename K, typename V> using Map = std::unordered_map<K, V>;
template<typename T> using Set = std::unordered_set<T>;

enum class WeightFormat {
  UNWEIGHTED,
  MINIC2D,
  CACHET, // cachet bug: '-1' weight
  WCNF, // 'w' line: trailing '0' is optional
  WPCNF // 'vp' line: trailing '0' is optional
};

enum class PlanningStrategy { FIRST_JOIN_TREE, TIMING };

enum class ClusteringHeuristic { BUCKET_LIST, BUCKET_TREE, BOUQUET_LIST, BOUQUET_TREE };

enum class VarOrderingHeuristic { APPEARANCE, DECLARATION, RANDOM, MCS, LEXP, LEXM, MINFILL };

enum class DdPackage { CUDD, SYLVAN };

enum class JoinPriority { ARBITRARY, SMALLEST_FIRST, LARGEST_FIRST };

/* global variables ***********************************************************/

extern PlanningStrategy planningStrategy;
extern DdPackage ddPackage;
extern JoinPriority joinPriority;
extern Int dotFileIndex;
extern Int randomSeed; // for reproducibility
extern Int verbosityLevel;
extern TimePoint startTime;

/* constants ******************************************************************/

extern const string &BOLD_LINE;
extern const string &THICK_LINE;
extern const string &THIN_LINE;

extern const string &PROBLEM_WORD;

extern const string &STDIN_CONVENTION;

extern const string &REQUIRED_OPTION_GROUP;
extern const string &OPTIONAL_OPTION_GROUP;

extern const string &HELP_OPTION;
extern const string &CNF_FILE_OPTION;
extern const string &WEIGHT_FORMAT_OPTION;
extern const string &OUTPUT_WEIGHT_FORMAT_OPTION;
extern const string &JT_FILE_OPTION;
extern const string &PLANNING_STRATEGY_OPTION;
extern const string &JT_WAIT_DURAION_OPTION;
extern const string &PERFORMANCE_FACTOR_OPTION; // alpha in parallel-tensor paper
extern const string &OUTPUT_FORMAT_OPTION;
extern const string &CLUSTERING_HEURISTIC_OPTION;
extern const string &CLUSTER_VAR_ORDER_OPTION;
extern const string &DD_VAR_ORDER_OPTION;
extern const string &DD_PACKAGE_OPTION;
extern const string &WORKER_COUNT_OPTION;
extern const string &JOIN_PRIORITY_OPTION;
extern const string &RANDOM_SEED_OPTION;
extern const string &VERBOSITY_LEVEL_OPTION;

extern const std::map<Int, WeightFormat> WEIGHT_FORMAT_CHOICES;
extern const Int DEFAULT_WEIGHT_FORMAT_CHOICE;

extern const std::map<string, PlanningStrategy> PLANNING_STRATEGY_CHOICES;
extern const string DEFAULT_PLANNING_STRATEGY_CHOICE;

extern const Float DEFAULT_JT_WAIT_SECONDS;
extern const Float DEFAULT_PERFORMANCE_FACTOR;

extern const std::map<Int, ClusteringHeuristic> CLUSTERING_HEURISTIC_CHOICES;
extern const Int DEFAULT_CLUSTERING_HEURISTIC_CHOICE;

extern const std::map<Int, VarOrderingHeuristic> VAR_ORDERING_HEURISTIC_CHOICES;
extern const Int DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE;
extern const Int DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE;

extern const std::map<Int, DdPackage> DD_PACKAGE_CHOICES;
extern const Int DEFAULT_DD_PACKAGE_CHOICE;

extern const Int DEFAULT_WORKER_COUNT;

extern const std::map<string, JoinPriority> JOIN_PRIORITY_CHOICES;
extern const string DEFAULT_JOIN_PRIORITY_CHOICE;

extern const Int DEFAULT_RANDOM_SEED;

extern const vector<Int> VERBOSITY_LEVEL_CHOICES;
extern const Int DEFAULT_VERBOSITY_LEVEL_CHOICE;

extern const Float NEGATIVE_INFINITY;

extern const Int DUMMY_MIN_INT;
extern const Int DUMMY_MAX_INT;

extern const string &DOT_DIR; // dir for .dot files (DDs)

/* namespaces *****************************************************************/

namespace util {
  bool isInt(Float f);

  /* functions: printing ******************************************************/

  void printComment(const string &message, Int preceedingNewLines = 0, Int followingNewLines = 1, bool commented = true);
  void printSolutionLine(Float modelCount, Int preceedingThinLines = 1, Int followingThinLines = 1);

  void printBoldLine(bool commented);
  void printThickLine(bool commented = true);
  void printThinLine(std::ostream &outputStream = cout);

  void printHelpOption();
  void printCnfFileOption();
  void printWeightFormatOption();
  void printOutputWeightFormatOption();
  void printJtFileOption();
  void printPlanningStrategyOption();
  void printJtWaitOption();
  void printPerformanceFactorOption();
  void printClusteringHeuristicOption();
  void printCnfVarOrderingHeuristicOption();
  void printDdVarOrderingHeuristicOption();
  void printDdPackageOption();
  void printWorkerCountOption();
  void printJoinPriorityOption();
  void printRandomSeedOption();
  void printVerbosityLevelOption();

  /* functions: argument parsing **********************************************/

  void printArgv(int argc, char *argv[]);

  string getWeightFormatName(WeightFormat weightFormat);
  string getPlanningStrategyName(PlanningStrategy planningStrategy);
  string getClusteringHeuristicName(ClusteringHeuristic clusteringHeuristic);
  string getVarOrderingHeuristicName(VarOrderingHeuristic varOrderingHeuristic);
  string getDdPackageName(DdPackage ddPackage);
  string getJoinPriorityName(JoinPriority joinPriority);
  string getVerbosityLevelName(Int verbosityLevel);

  /* functions: cnf ***********************************************************/

  Int getCnfVar(Int literal);
  Set<Int> getClauseCnfVars(const vector<Int> &clause);
  Set<Int> getClusterCnfVars(const vector<Int> &cluster, const vector<vector<Int>> &clauses);

  bool appearsIn(Int cnfVar, const vector<Int> &clause);
  bool isPositiveLiteral(Int literal);

  Int getLiteralRank(Int literal, const vector<Int> &cnfVarOrdering);
  Int getMinClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering);
  Int getMaxClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering);

  void printClause(const vector<Int> &clause);
  void printCnf(const vector<vector<Int>> &clauses);
  void printLiteralWeights(const Map<Int, Float> &literalWeights);

  /* functions: timing ********************************************************/

  TimePoint getTimePoint();
  Float getSeconds(TimePoint startTime);
  void printDuration(TimePoint startTime);

  void handleSignal(int signal); // `timeout` sends SIGTERM

  /* functions: error handling ************************************************/

  void showWarning(const string &message, bool commented = true);
  void showError(const string &message, bool commented = true);

  /* functions: templates implemented in headers to avoid linker errors *******/

  template<typename T> void printRow(const string &name, const T &value) {
    cout << "c " << std::left << std::setw(30) << name;
    cout << value << "\n";
  }

  template<typename T> void printContainer(const T &container) {
    cout << "printContainer:\n";
    for (const T &member : container) {
      cout << "\t" << member << "\t";
    }
    cout << "\n";
  }

  template<typename K, typename V> void printMap(const Map<K, V> &m) {
    cout << "printMap:\n";
    for (const auto &kv : m) {
      cout << "\t" << kv.first << "\t:\t" << kv.second << "\n";
    }
    cout << "\n";
  }

  template<typename Key, typename Value> bool isLessValued(std::pair<Key, Value> a, std::pair<Key, Value> b) {
    return a.second < b.second;
  }

  template<typename T> T getSoleMember(const vector<T> &v) {
    if (v.size() != 1) showError("vector is not singleton");
    return v.at(0);
  }

  template<typename T> void popBack(T &element, vector<T> &v) {
    if (v.empty()) showError("vector is empty");
    element = v.back();
    v.pop_back();
  }

  template<typename T> void invert(T &t) {
    std::reverse(t.begin(), t.end());
  }

  template<typename T, typename U> bool isFound(const T &element, const U &container) {
    return std::find(std::begin(container), std::end(container), element) != std::end(container);
  }

  template<typename T, typename U> Int getIndex(const T &element, const U &container) {
    return std::distance(std::begin(container), std::find(std::begin(container), std::end(container), element));
  }

  template<typename T> Set<T> getIntersection(const Set<T> &container1, const Set<T> &container2) {
    Set<T> intersection;
    for (const T &member : container1) {
      if (isFound(member, container2)) {
        intersection.insert(member);
      }
    }
    return intersection;
  }

  template<typename T, typename U1, typename U2> void differ(Set<T> &diff, const U1 &members, const U2 &nonmembers) {
    for (const auto &member : members) {
      if (!isFound(member, nonmembers)) {
        diff.insert(member);
      }
    }
  }

  template<typename T, typename U> void unionize(Set<T> &unionSet, const U &container) {
    for (const auto &member : container) {
      unionSet.insert(member);
    }
  }

  template<typename T> Set<T> getUnion(const vector<Set<T>> &containers) {
    Set<T> set;
    for (const Set<T> &container : containers) {
      unionize(set, container);
    }
    return set;
  }

  template<typename T, typename U> bool isDisjoint(const T &container, const U &container2) {
    for (const auto &member : container) {
      for (const auto &member2 : container2) {
        if (member == member2) {
          return false;
        }
      }
    }
    return true;
  }

  template<typename T> Float adjustModelCount(Float apparentModelCount, const T &projectedCnfVars, const Map<Int, Float> &literalWeights) {
    Float totalModelCount = apparentModelCount;

    Int totalLiteralCount = literalWeights.size();
    if (totalLiteralCount % 2 == 1) showError("odd total literal count");

    Int totalVarCount = totalLiteralCount / 2;
    if (totalVarCount < projectedCnfVars.size()) showError("more projected vars than total vars");

    for (Int cnfVar = 1; cnfVar <= totalVarCount; cnfVar++) {
      if (!isFound(cnfVar, projectedCnfVars)) {
        totalModelCount *= literalWeights.at(cnfVar) + literalWeights.at(-cnfVar);
      }
    }

    if (totalModelCount == 0) {
      showWarning("floating-point underflow may have occured");
    }
    return totalModelCount;
  }

  template<typename T> void shuffleRandomly(T &container) {
    std::mt19937 generator;
    generator.seed(randomSeed);
    std::shuffle(container.begin(), container.end(), generator);
  }

  template<typename Diagram> Set<Int> getSupport(const Diagram &dd) {
    Set<Int> support;
    for (Int ddVar : dd.SupportIndices()) support.insert(ddVar);
    return support;
  }

  template<typename Diagram> Set<Int> getSupportSuperset(const vector<Diagram> &dds) {
    Set<Int> supersupport;
    for (const Diagram &dd : dds) for (Int var : dd.SupportIndices()) supersupport.insert(var);
    return supersupport;
  }

  template<typename Diagram> Int getMinDdRank(const Diagram &dd, const vector<Int> &ddVarToCnfVarMap, const vector<Int> &cnfVarOrdering) {
    Int minRank = DUMMY_MAX_INT;
    for (Int ddVar : getSupport(dd)) {
      Int cnfVar = ddVarToCnfVarMap.at(ddVar);
      Int rank = getLiteralRank(cnfVar, cnfVarOrdering);
      minRank = std::min(minRank, rank);
    }
    return minRank;
  }

  template<typename Diagram> Int getMaxDdRank(const Diagram &dd, const vector<Int> &ddVarToCnfVarMap, const vector<Int> &cnfVarOrdering) {
    Int maxRank = DUMMY_MIN_INT;
    for (Int ddVar : getSupport(dd)) {
      Int cnfVar = ddVarToCnfVarMap.at(ddVar);
      Int rank = getLiteralRank(cnfVar, cnfVarOrdering);
      maxRank = std::max(maxRank, rank);
    }
    return maxRank;
  }
}

namespace parallelizing {
  void initializeSylvan(Int workerCount); // automatically detects number of threads if workerCount = 0
  void testSylvan();
  void quitSylvan();
}

/* classes ********************************************************************/

class Dd { // wrapper for CUDD and Sylvan
  static Cudd mgr; // CUDD manager

  ADD cuadd; // CUDD ADD
  Mtbdd mtbdd; // Sylvan ADD

public:
  static void testPriorityQueue(std::priority_queue<Dd, vector<Dd>> &dds); // prints and clears dds
  static Dd getConstantDd(Float c);
  static Dd getDummyDd();
  static Dd getZeroDd();
  static Dd getOneDd();
  static Dd getVarDd(Int ddVar);
  static Dd getNegativeLiteralDd(Int ddVar);
  static Cudd getMgr();
  ADD getCuadd() const;
  Mtbdd getMtbdd() const;
  Dd(const ADD &cuadd);
  Dd(const Mtbdd &mtbdd);
  Dd(const Dd &add);

  Int countNodes() const;
  bool operator<(const Dd &right) const; // this = left < right (top of priotity queue is rightmost element)
  Float getTerminalValue() const;
  Float countConstDdFloat() const;
  Dd getDisjunction(const Dd &add) const;
  Dd getConjunction(const Dd &add) const;
  Dd getComposition(Int ddVar, bool isTrue) const;
  Dd getProduct(const Dd &add) const;
  Dd getMax(const Dd &add) const; // float max (not bool max)
  Dd getSum(const Dd &add) const;
  Set<Int> getSupport() const;
  Dd getAbstraction(
    Int ddVar,
    const vector<Int> &ddVarToCnfVarMap,
    const Map<Int, Float> &literalWeights,
    bool additive = true // getSum else getMax
  ) const;
  void writeDotFile(const std::string &dotFileDir = DOT_DIR) const;
};

class MyError {
public:
  MyError(const string &message, bool commented);
};
