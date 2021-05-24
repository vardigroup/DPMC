#pragma once

/* inclusions =============================================================== */

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <signal.h>
#include <sys/time.h>
#include <thread>
#include <unordered_set>

#include <gmpxx.h>

/* uses ===================================================================== */

using std::cout;
using std::greater;
using std::istream_iterator;
using std::left;
using std::map;
using std::max;
using std::min;
using std::multimap;
using std::mutex;
using std::next;
using std::ostream;
using std::pair;
using std::right;
using std::setprecision;
using std::setw;
using std::string;
using std::thread;
using std::to_string;
using std::vector;

/* types ==================================================================== */

using Float = long double;
using Int = long long;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

template<typename K, typename V> using Map = std::unordered_map<K, V>;
template<typename T> using Set = std::unordered_set<T>;

/* consts =================================================================== */

const Int MIN_INT = std::numeric_limits<Int>::min();
const Int MAX_INT = std::numeric_limits<Int>::max();

const Float INF = std::numeric_limits<Float>::infinity();

const string JT_WORD = "jt";
const string VAR_ELIM_WORD = "e";

const string WARNING = "c MY_WARNING: ";
const string THIN_LINE = "c ------------------------------------------------------------------\n";

const string CNF_FILE_OPTION = "cf";
const string PROJECTED_COUNTING_OPTION = "pc";
const string DD_PACKAGE_OPTION = "dp";
const string RANDOM_SEED_OPTION = "rs";
const string VERBOSE_CNF_OPTION = "vc";
const string VERBOSE_SOLVING_OPTION = "vs";

const string CUDD = "c";
const string SYLVAN = "s";
const map<string, string> DD_PACKAGES = {
  {CUDD, "CUDD"},
  {SYLVAN, "SYLVAN"}
};

const Int RANDOM = 0;
const Int DECLARED = 1;
const Int MOST_CLAUSES = 2;
const Int MINFILL = 3;
const Int MCS = 4;
const Int LEXP = 5;
const Int LEXM = 6;
const map<Int, string> CNF_VAR_ORDER_HEURISTICS = {
  {RANDOM, "RANDOM"},
  {DECLARED, "DECLARED"},
  {MOST_CLAUSES, "MOST_CLAUSES"},
  {MINFILL, "MINFILL"},
  {MCS, "MCS"},
  {LEXP, "LEXP"},
  {LEXM, "LEXM"}
};

const Int BIGGEST_NODE = 7;
const Int HIGHEST_NODE = 8;
const map<Int, string> JOIN_TREE_VAR_ORDER_HEURISTICS = {
  {BIGGEST_NODE, "BIGGEST_NODE"},
  {HIGHEST_NODE, "HIGHEST_NODE"}
};

const string BUCKET_LIST = "bel";
const string BUCKET_TREE = "bet";
const string BOUQUET_LIST = "bml";
const string BOUQUET_TREE = "bmt";
const map<string, string> CLUSTERING_HEURISTICS = {
  {BUCKET_LIST, "BUCKET_ELIM_LIST"},
  {BUCKET_TREE, "BUCKET_ELIM_TREE"},
  {BOUQUET_LIST, "BOUQUET_METHOD_LIST"},
  {BOUQUET_TREE, "BOUQUET_METHOD_TREE"}
};

const Int PARSED_INPUT = 1;
const Int RAW_INPUT = 2;
const string INPUT_VERBOSITIES = to_string(PARSED_INPUT) + ", " + to_string(RAW_INPUT) + "; int";

/* global vars ============================================================== */

extern TimePoint toolStartPoint;

extern bool weightedCounting;
extern bool projectedCounting;
extern Int randomSeed; // for reproducibility
extern bool multiplePrecision;
extern bool logCounting; // implies !multiplePrecision
extern Int verboseCnf; // 1: parsed cnf, 2: raw cnf too
extern Int verboseSolving; // 0: solution, 1: pasred options too, 2: more info

/* namespaces =============================================================== */

namespace util {
  string useDdPackage(string ddPackageArg);

  map<Int, string> getVarOrderHeuristics();
  string helpVarOrderHeuristic(string prefix);
  string helpVerboseSolving();

  TimePoint getTimePoint();
  Float getDuration(TimePoint start); // in seconds

  vector<string> splitInputLine(string line);
  void printInputLine(string line, Int lineIndex);

  void printRowKey(string key, size_t keyWidth);
  void printPreciseFloat(Float f);
  void printPreciseFloatRow(string key, Float f, size_t keyWidth);

  template<typename T> void printRow(string key, const T& val, size_t keyWidth = 30) {
    printRowKey(key, keyWidth);
    cout << val << "\n";
  }

  template<typename T, typename U> pair<U, T> flipPair(const pair<T, U>& p) {
    return pair<U, T>(p.second, p.first);
  }

  template<typename T, typename U> multimap<U, T, greater<U>> flipMap(const Map<T, U>& inMap) { // decreasing
    multimap<U, T, greater<U>> outMap;
    transform(inMap.begin(), inMap.end(), inserter(outMap, outMap.begin()), flipPair<T, U>);
    return outMap;
  }

  template<typename T, typename U> bool isFound(const T& element, const vector<U>& container) {
    return std::find(begin(container), end(container), element) != end(container);
  }

  template<typename T> Set<T> getIntersection(const Set<T>& container1, const Set<T>& container2) {
    Set<T> intersection;
    for (const T& member : container1) {
      if (container2.contains(member)) {
        intersection.insert(member);
      }
    }
    return intersection;
  }

  template<typename T, typename U> Set<T> getDiff(const Set<T>& members, const U& nonmembers) {
    Set<T> diff;
    for (const T& member : members) {
      if (!nonmembers.contains(member)) {
        diff.insert(member);
      }
    }
    return diff;
  }

  template<typename T, typename U> void unionize(Set<T>& unionSet, const U& container) {
    for (const auto& member : container) {
      unionSet.insert(member);
    }
  }

  template<typename T> Set<T> getUnion(const vector<Set<T>>& containers) {
    Set<T> s;
    for (const Set<T>& container : containers) {
      unionize(s, container);
    }
    return s;
  }

  template<typename T> bool isDisjoint(const Set<T>& container1, const Set<T>& container2) {
    for (const T& member : container1) {
      if (container2.contains(member)) {
        return false;
      }
    }
    return true;
  }
}

/* classes for exceptions =================================================== */

class EmptyClauseException : public std::exception {
public:
  EmptyClauseException(Int lineIndex, string line);
};

class MyError : public std::exception {
public:
  template<typename ... Ts> MyError(const Ts& ... args) { // en.cppreference.com/w/cpp/language/fold
    cout << "\n";
    cout << "c ******************************************************************\n";
    cout << "c MY_ERROR: ";
    (cout << ... << args); // fold expression
    cout << "\n";
  }
};

/* classes for cnf formulas ================================================= */

class Number {
public:
  mpq_class quotient;
  Float fraction;

  Number(const mpq_class& q); // multiplePrecision
  Number(Float f); // !multiplePrecision
  Number(const Number& n);
  Number(string s = "0"); // `s` is "{int1}/{int2}" or "{float1}"

  Float getLogSumExp(const Number& n) const; // logCounting
  Float getLog10() const;
  bool operator==(const Number& n) const;
  bool operator!=(const Number& n) const;
  bool operator<(const Number& n) const;
  bool operator<=(const Number& n) const;
  bool operator>=(const Number& n) const;
  Number operator*(const Number& n) const;
  Number& operator*=(const Number& n);
  Number operator+(const Number& n) const;
  Number& operator+=(const Number& n);
  Number operator-(const Number& n) const;
};

class Graph { // undirected
public:
  Set<Int> vertices;
  Map<Int, Set<Int>> adjacencyMap;

  Graph(const Set<Int>& vs);
  void addEdge(Int v1, Int v2);

  bool isNeighbor(Int v1, Int v2) const;
  bool hasPath(Int from, Int to, Set<Int>& visitedVertices) const; // path length >= 0
  bool hasPath(Int from, Int to) const;
  void removeVertex(Int v); // also removes edges from/to v
  void fillInEdges(Int v); // does not remove v
  Int countFillInEdges(Int v) const;
  Int getMinfillVertex() const;
};

class Label : public vector<Int> { // for lexicographic search
public:
  void addNumber(Int i); // retains descending order
  static bool hasSmallerLabel(const pair<Int, Label>& a, const pair <Int, Label>& b);
};

class Clause : public Set<Int> {
public:
  void printClause() const;
  Set<Int> getClauseVars() const;
};

class Cnf {
public:
  vector<Clause> clauses;
  Int declaredVarCount = 0;
  Set<Int> apparentVars; // as opposed to hidden vars that are declared but appear in no clause
  Set<Int> additiveVars; // as opposed to existential vars
  Map<Int, Number> literalWeights; // for additive and disjunctive vars
  Map<Int, Set<Int>> varToClauses; // var |-> clause indices

  void printClauses() const;
  void printLiteralWeights() const;
  Set<Int> getDisjunctiveVars() const;

  void addClause(const Clause& clause);
  void setApparentVars();
  Graph getPrimalGraph() const;
  vector<Int> getRandomVarOrder() const;
  vector<Int> getDeclaredVarOrder() const;
  vector<Int> getMostClausesVarOrder() const;
  vector<Int> getMinfillVarOrder() const;
  vector<Int> getMcsVarOrder() const;
  vector<Int> getLexpVarOrder() const;
  vector<Int> getLexmVarOrder() const;
  vector<Int> getCnfVarOrder(Int cnfVarOrderHeuristic) const;

  Cnf(); // empty conjunction
  Cnf(string filePath);
};

/* classes for join trees =================================================== */

class Assignment : public Map<Int, bool> { // partial var assignment
public:
  Assignment();
  Assignment(Int var, bool val);

  void printAssignment() const;
  static vector<Assignment> extendAssignments(const vector<Assignment>& assignments, Int var);
};

class JoinNode { // abstract
public:
  static Int nodeCount;
  static Int terminalCount;
  static Set<Int> nonterminalIndices;

  static Int backupNodeCount;
  static Int backupTerminalCount;
  static Set<Int> backupNonterminalIndices;

  static Cnf cnf; // this field must be set exactly once before any JoinNode object is constructed

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

  vector<Assignment> getAdditiveAssignments(Int varOrderHeuristic, Int sliceVarCount) const;

  JoinNonterminal(
    const vector<JoinNode*>& children,
    const Set<Int>& projectionVars = Set<Int>(),
    Int requestedNodeIndex = MIN_INT
  );
};

/* global functions ========================================================= */

ostream& operator<<(ostream& os, const Number& n);
