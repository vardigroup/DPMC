#pragma once

/* inclusions =============================================================== */

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <random>
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

enum class WeightedCountingMode {
  NO_VARS = 0,
  ALL_VARS = 1,
  OUTER_VARS = 2
};

/* consts =================================================================== */

const Float INF = std::numeric_limits<Float>::infinity();

const Int MIN_INT = std::numeric_limits<Int>::min();
const Int MAX_INT = std::numeric_limits<Int>::max();

const string JOIN_TREE_WORD = "jt";
const string ELIM_VARS_WORD = "e";

const string WARNING = "c MY_WARNING: ";
const string DASH_LINE = "c ------------------------------------------------------------------\n";

const string CNF_FILE_OPTION = "cf";
const string PROJECTED_COUNTING_OPTION = "pc";
const string DD_PACKAGE_OPTION = "dp";
const string RANDOM_SEED_OPTION = "rs";
const string VERBOSE_CNF_OPTION = "vc";
const string VERBOSE_SOLVING_OPTION = "vs";
const string HELP_OPTION = "h";

/* diagram packages: */
const string CUDD_PACKAGE = "c";
const string SYLVAN_PACKAGE = "s";
const map<string, string> DD_PACKAGES = {
  {CUDD_PACKAGE, "CUDD"},
  {SYLVAN_PACKAGE, "SYLVAN"}
};

/* CNF var order heuristics: */
const Int RANDOM_HEURISTIC = 0;
const Int DECLARATION_HEURISTIC = 1;
const Int MOST_CLAUSES_HEURISTIC = 2;
const Int MIN_FILL_HEURISTIC = 3;
const Int MCS_HEURISTIC = 4;
const Int LEX_P_HEURISTIC = 5;
const Int LEX_M_HEURISTIC = 6;
const map<Int, string> CNF_VAR_ORDER_HEURISTICS = {
  {RANDOM_HEURISTIC, "RANDOM"},
  {DECLARATION_HEURISTIC, "DECLARATION"},
  {MOST_CLAUSES_HEURISTIC, "MOST_CLAUSES"},
  {MIN_FILL_HEURISTIC, "MIN_FILL"},
  {MCS_HEURISTIC, "MCS"},
  {LEX_P_HEURISTIC, "LEX_P"},
  {LEX_M_HEURISTIC, "LEX_M"}
};

/* JT var order heuristics: */
const Int BIGGEST_NODE_HEURISTIC = 7;
const Int HIGHEST_NODE_HEURISTIC = 8;
const map<Int, string> JOIN_TREE_VAR_ORDER_HEURISTICS = {
  {BIGGEST_NODE_HEURISTIC, "BIGGEST_NODE"},
  {HIGHEST_NODE_HEURISTIC, "HIGHEST_NODE"}
};

/* clustering heuristics: */
const string BUCKET_ELIM_LIST = "bel";
const string BUCKET_ELIM_TREE = "bet";
const string BOUQUET_METHOD_LIST = "bml";
const string BOUQUET_METHOD_TREE = "bmt";
const map<string, string> CLUSTERING_HEURISTICS = {
  {BUCKET_ELIM_LIST, "BUCKET_ELIM_LIST"},
  {BUCKET_ELIM_TREE, "BUCKET_ELIM_TREE"},
  {BOUQUET_METHOD_LIST, "BOUQUET_METHOD_LIST"},
  {BOUQUET_METHOD_TREE, "BOUQUET_METHOD_TREE"}
};

/* global vars ============================================================== */

extern WeightedCountingMode weightedCountingMode;
extern bool projectedCounting;
extern Int randomSeed; // for reproducibility
extern bool multiplePrecision;
extern Int verboseCnf; // 1: stats, 2: parsed CNF too, 3: raw CNF too
extern Int verboseSolving; // 0: solution, 1: parsed options too, 2: more info

extern TimePoint toolStartPoint;

/* namespaces =============================================================== */

namespace util {
  vector<Int> getSortedNums(const Set<Int>& nums);

  map<Int, string> getVarOrderHeuristics();

  string helpVarOrderHeuristic(const map<Int, string>& heuristics);
  string helpVerboseCnfProcessing();
  string helpVerboseSolving();

  TimePoint getTimePoint();
  Float getDuration(TimePoint start); // in seconds

  vector<string> splitInputLine(const string& line);
  void printInputLine(const string& line, Int lineIndex);

  void printRowKey(const string& key, size_t keyWidth);

  template<typename T> void printRow(const string& key, const T& val, size_t keyWidth = 32) {
    printRowKey(key, keyWidth);

    Int p = cout.precision();
    // cout.precision(std::numeric_limits<Float>::digits10); // default for Float: 6 digits
    cout << val << "\n";
    cout.precision(p);
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

  template<typename T, typename U> Set<T> getDiff(const Set<T>& members, const U& nonMembers) {
    Set<T> diff;
    for (const T& member : members) {
      if (!nonMembers.contains(member)) {
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

class UnsatException : public std::exception {};

class EmptyClauseException : public UnsatException {
public:
  EmptyClauseException(Int lineIndex, const string& line);
};

class UnsatSolverException : public UnsatException {
public:
  UnsatSolverException();
};

class MyError : public std::exception {
public:
  template<typename ... Ts> MyError(const Ts& ... args) { // en.cppreference.com/w/cpp/language/fold
    cout << "\n";
    cout << "c ******************************************************************\n";
    cout << "c MY_ERROR: ";
    (cout << ... << args); // fold expression
    cout << "\n";
    cout << "c ******************************************************************\n";
  }
};

/* classes for CNF formulas ================================================= */

class Number {
public:
  mpq_class quotient;
  Float fraction;

  Number(const mpq_class& q); // multiplePrecision
  Number(Float f); // !multiplePrecision
  Number(const Number& n);
  Number(const string& repr = "0"); // `repr` is `<int>/<int>` or `<float>`

  Number getAbsolute() const;
  Float getLog10() const;
  Float getLogSumExp(const Number& n) const;
  bool operator==(const Number& n) const;
  bool operator!=(const Number& n) const;
  bool operator<(const Number& n) const;
  bool operator<=(const Number& n) const;
  bool operator>(const Number& n) const;
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
  void removeVertex(Int v); // also removes edges from and to `v`
  void fillInEdges(Int v); // does not remove `v`
  Int getFillInEdgeCount(Int v) const;
  Int getMinFillVertex() const;
};

class Label : public vector<Int> { // for lexicographic search
public:
  void addNumber(Int i); // retains descending order
  static bool hasSmallerLabel(const pair<Int, Label>& a, const pair <Int, Label>& b);
};

class Clause : public Set<Int> {
public:
  bool xorFlag;

  Clause(bool xorFlag);

  void insertLiteral(Int literal);

  void printClause() const;
  Set<Int> getClauseVars() const;
};

class Cnf {
public:
  Int declaredVarCount = 0;
  Set<Int> outerVars;
  Map<Int, Number> literalWeights; // for outer and inner vars
  vector<Clause> clauses;
  Int xorClauseCount = 0;

  Set<Int> apparentVars; // as opposed to hidden vars that are declared but appear in no clause
  Map<Int, Set<Int>> varToClauses; // apparent var |-> clause indices

  Set<Int> getInnerVars() const;
  Map<Int, Number> getUnprunableWeights() const;

  static void printLiteralWeight(Int literal, const Number& weight);
  void printLiteralWeights() const;
  void printClauses() const;

  void addClause(const Clause& clause);
  void setApparentVars();
  Graph getPrimalGraph() const;
  vector<Int> getRandomVarOrder() const;
  vector<Int> getDeclarationVarOrder() const;
  vector<Int> getMostClausesVarOrder() const;
  vector<Int> getMinFillVarOrder() const;
  vector<Int> getMcsVarOrder() const;
  vector<Int> getLexPVarOrder() const;
  vector<Int> getLexMVarOrder() const;
  vector<Int> getCnfVarOrder(Int cnfVarOrderHeuristic) const;

  bool isMc21ShowLine(const vector<string> &words) const; // c p show <vars> [0]
  bool isMc21WeightLine(const vector<string> &words) const; // c p weight <literal> <weight> [0]

  void setDefaultLiteralWeights(Int var);
  void completeImplicitLiteralWeight(Int literal);
  void completeLiteralWeights();

  void printStats() const;

  void readCnfFile(const string& filePath);

  Cnf(); // empty conjunction
};

/* classes for join trees =================================================== */

class Assignment : public Map<Int, bool> { // partial var assignment
public:
  Assignment();
  Assignment(Int var, bool val);
  Assignment(const string& bitString);

  bool getValue(Int var) const; // returns `true` if `var` is unassigned
  void printAssignment() const;
  static vector<Assignment> getExtendedAssignments(const vector<Assignment>& assignments, Int var);
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

  static void resetStaticFields(); // backs up and re-initializes static fields
  static void restoreStaticFields(); // from backup

  virtual Int getWidth(const Assignment& assignment = Assignment()) const = 0; // of subtree

  virtual void updateVarSizes(
    Map<Int, size_t>& varSizes // var x |-> size of biggest node containing x
  ) const = 0;

  Set<Int> getPostProjectionVars() const;
  Int chooseClusterIndex(
    Int clusterIndex, // of this node
    const vector<Set<Int>>& projectableVarSets, // Z_1, ..., Z_m
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
  void printNode(const string& startWord) const; // 1-indexing
  void printSubtree(const string& startWord = "") const; // post-order traversal

  Int getWidth(const Assignment& assignment = Assignment()) const override;

  void updateVarSizes(Map<Int, size_t>& varSizes) const override;
  vector<Int> getBiggestNodeVarOrder() const;
  vector<Int> getHighestNodeVarOrder() const;
  vector<Int> getVarOrder(Int varOrderHeuristic) const;

  vector<Assignment> getOuterAssignments(Int varOrderHeuristic, Int sliceVarCount) const;

  JoinNonterminal(
    const vector<JoinNode*>& children,
    const Set<Int>& projectionVars = Set<Int>(),
    Int requestedNodeIndex = MIN_INT
  );
};

/* global functions ========================================================= */

ostream& operator<<(ostream& stream, const Number& n);
ostream& operator<<(ostream& stream, WeightedCountingMode mode);
