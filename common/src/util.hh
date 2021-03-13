/* src inclusion chain: util.h in logic.h in (dmc|htb).h */

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
#include <signal.h>
#include <sys/time.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <gmpxx.h>

/* uses ===================================================================== */

using std::addressof;
using std::cout;
using std::cref;
using std::istream_iterator;
using std::istream;
using std::left;
using std::map;
using std::max;
using std::min;
using std::mutex;
using std::next;
using std::ostream;
using std::pair;
using std::ref;
using std::right;
using std::setprecision;
using std::setw;
using std::string;
using std::thread;
using std::to_string;
using std::vector;

/* types ==================================================================== */

using Float = long double; // 16 bytes
using Int = long long; // 8 bytes
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>; // 8 bytes

template<typename K, typename V> using Map = std::unordered_map<K, V>;
template<typename T> using Set = std::unordered_set<T>;

/* global vars ============================================================== */

extern string weightFormat;
extern Int randomSeed; // for reproducibility
extern bool multiplePrecision;
extern bool logCounting;
extern Int verbosityLevel;

extern TimePoint startTime;
extern Int dotFileIndex;

/* consts =================================================================== */

const string THREAD_COUNT_OPTION = "tc";
const string DD_PACKAGE_OPTION = "dp";
const string RANDOM_SEED_OPTION = "rs";
const string VERBOSITY_LEVEL_OPTION = "vl";

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
const map<Int, string> JT_VAR_ORDER_HEURISTICS = {
  {BIGGEST_NODE, "BIGGEST_NODE"},
  {HIGHEST_NODE, "HIGHEST_NODE"}
};

const map<Int, string> VERBOSITY_LEVELS = {
  {0, "SOLUTION"},
  {1, "PARSED_COMMAND"},
  {2, "CLAUSES_AND_WEIGHTS"},
  {3, "INPUT_LINES"}
};

const Int MIN_INT = std::numeric_limits<Int>::min();
const Int MAX_INT = std::numeric_limits<Int>::max();

const Float INF = std::numeric_limits<Float>::infinity();

const string THICK_LINE = "c ==================================================================\n";
const string THIN_LINE = "c ------------------------------------------------------------------\n";

const string WARNING = "c MY_WARNING: ";

/* namespaces =============================================================== */

namespace util {
  TimePoint getTimePoint();
  Float getSeconds(TimePoint start);

  vector<string> splitWords(string line);
  void printInputLine(Int lineIndex, string line);

  map<Int, string> getVarOrderHeuristics();
  string helpVarOrderHeuristic(string prefix);
  string helpVerbosity();

  void checkTypeSizes();

  void handleSignal(int signal); // reports OS signals
  void setSignalHandler();

  /* functions: templates implemented in headers to avoid linker errors ===== */

  template<typename T> void printRow(string key, const T& val, string startWord = "c") {
    cout << startWord << " " << left << setw(25) << key << " " << val << "\n";
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

/* classes ================================================================== */

class MyError {
public:
  template<typename ... Ts> MyError(const Ts& ... args) { // en.cppreference.com/w/cpp/language/fold
    cout << "\n";
    cout << "c ******************************************************************\n";
    cout << "c MY_ERROR: ";
    (cout << ... << args); // fold expression
    cout << "\n";
  }
};

class Number {
public:
  mpq_class quotient;
  Float fraction;

  Number(const mpq_class& q); // multiplePrecision
  Number(Float f); // !multiplePrecision
  Number(const Number& n);
  Number(string s = "0.0");

  Float getFloat() const;
  Float getLn() const; // logCounting
  bool operator==(const Number& n) const;
  bool operator!=(const Number& n) const;
  Number operator*(const Number& n) const;
  Number& operator*=(const Number& n);
  Number operator+(const Number& n) const;
  Number& operator+=(const Number& n);
  Number operator-(const Number& n) const;
};

/* global functions ========================================================= */

ostream& operator<<(ostream& os, const Number& n);
