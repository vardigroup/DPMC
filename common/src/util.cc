/* inclusions =============================================================== */

#include "util.hh"

/* global vars ============================================================== */

string weightFormat;
Int randomSeed;
bool multiplePrecision;
bool logCounting;
Int verbosityLevel;

TimePoint startTime;
Int dotFileIndex = 1;

/* namespaces =============================================================== */

/* namespace util =========================================================== */

TimePoint util::getTimePoint() {
  return std::chrono::steady_clock::now();
}

Float util::getSeconds(TimePoint start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(getTimePoint() - start).count() / 1000.0;
}

vector<string> util::splitWords(string line) {
  std::istringstream inputStringStream(line);
  vector<string> words;
  copy(istream_iterator<string>(inputStringStream), istream_iterator<string>(), back_inserter(words));
  return words;
}

void util::printInputLine(Int lineIndex, string line) {
  cout << "c line " << right << setw(5) << lineIndex << ": " << line << "\n";
}

map<Int, string> util::getVarOrderHeuristics() {
  map<Int, string> m = CNF_VAR_ORDER_HEURISTICS;
  m.insert(JT_VAR_ORDER_HEURISTICS.begin(), JT_VAR_ORDER_HEURISTICS.end());
  return m;
}

string util::helpVarOrderHeuristic(string prefix) {
  map<Int, string> heuristics = CNF_VAR_ORDER_HEURISTICS;
  string s = prefix + " var order";

  if (prefix == "slice") {
    s += " [with " + THREAD_COUNT_OPTION + "_arg != 1 and " + DD_PACKAGE_OPTION + "_arg == " + CUDD + "]";
    heuristics = getVarOrderHeuristics();
  }
  else {
    assert(prefix == "diagram" || prefix == "cluster");
  }

  s += ": ";
  for (auto it = heuristics.begin(); it != heuristics.end(); it++) {
    s += to_string(it->first) + "/" + it->second;
    if (next(it) != heuristics.end()) {
      s += ", ";
    }
  }
  return s + " (negatives for inverse orders); int";
}

string util::helpVerbosity() {
  string s = "verbosity level: ";
  for (auto it = VERBOSITY_LEVELS.begin(); it != VERBOSITY_LEVELS.end(); it++) {
    s += to_string(it->first) + "/" + it->second;
    if (next(it) != VERBOSITY_LEVELS.end()) {
      s += ", ";
    }
  }
  return s + "; int";
}

void util::checkTypeSizes() {
  cout << "\n";
  cout << "type sizes in bytes:\n\n";

  cout << left << setw(20) << "float" << right << setw(5) << sizeof(float) << "\n";
  cout << left << setw(20) << "double" << right << setw(5) << sizeof(double) << "\n";
  cout << left << setw(20) << "long double" << right << setw(5) << sizeof(long double) << "\n\n";

  cout << left << setw(20) << "short" << right << setw(5) << sizeof(short) << "\n";
  cout << left << setw(20) << "int" << right << setw(5) << sizeof(int) << "\n";
  cout << left << setw(20) << "long" << right << setw(5) << sizeof(long) << "\n";
  cout << left << setw(20) << "long long" << right << setw(5) << sizeof(long long) << "\n\n";

  cout << left << setw(20) << "unsigned short" << right << setw(5) << sizeof(unsigned short) << "\n";
  cout << left << setw(20) << "unsigned Int" << right << setw(5) << sizeof(unsigned Int) << "\n";
  cout << left << setw(20) << "unsigned long" << right << setw(5) << sizeof(unsigned long) << "\n";
  cout << left << setw(20) << "unsigned long long" << right << setw(5) << sizeof(unsigned long long) << "\n\n";

  cout << left << setw(20) << "size_t" << right << setw(5) << sizeof(size_t) << "\n\n";

  cout << left << setw(20) << "TimePoint" << right << setw(5) << sizeof(TimePoint) << "\n";
}

void util::handleSignal(int signal) {
  cout << "\n";
  cout << WARNING << "exiting with OS signal " << signal << " after " << util::getSeconds(startTime) << "s\n";
  exit(signal);
}

void util::setSignalHandler() {
  signal(SIGINT, handleSignal); // Ctrl c
  signal(SIGTERM, handleSignal); // timeout
}

/* classes ================================================================== */

/* class Number ============================================================= */

Number::Number(const mpq_class& q) {
  assert(multiplePrecision);
  quotient = q;
}

Number::Number(Float f) {
  assert(!multiplePrecision);
  fraction = f;
}

Number::Number(const Number& n) {
  if (multiplePrecision) {
    quotient = n.quotient;
  }
  else {
    fraction = n.fraction;
  }
}

Number::Number(string s) {
  if (multiplePrecision) {
    *this = Number(mpq_class(mpf_class(s)));
  }
  else {
    *this = Number(stold(s));
  }
}

Float Number::getFloat() const {
  if (multiplePrecision) {
    return quotient.get_d();
  }
  return fraction;
}

Float Number::getLn() const {
  assert(logCounting);
  return log(fraction);
}

bool Number::operator==(const Number& n) const {
  if (multiplePrecision) {
    return quotient == n.quotient;
  }
  return fraction == n.fraction;
}

bool Number::operator!=(const Number& n) const {
  return !(*this == n);
}

Number Number::operator*(const Number& n) const {
  if (multiplePrecision) {
    return Number(quotient * n.quotient);
  }
  return Number(fraction * n.fraction);
}

Number& Number::operator*=(const Number& n) {
  *this = *this * n;
  return *this;
}

Number Number::operator+(const Number& n) const {
  if (multiplePrecision) {
    return Number(quotient + n.quotient);
  }
  return Number(fraction + n.fraction);
}

Number& Number::operator+=(const Number& n) {
  *this = *this + n;
  return *this;
}

Number Number::operator-(const Number& n) const {
  if (multiplePrecision) {
    return Number(quotient - n.quotient);
  }
  return Number(fraction - n.fraction);
}

/* global functions ========================================================= */

ostream& operator<<(ostream& os, const Number& n) {
  if (multiplePrecision) {
    os << n.quotient;
  }
  else {
    os << n.fraction;
  }
  return os;
}
