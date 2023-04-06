#include "common.hh"

/* global vars ============================================================== */

bool weightedCounting;
bool projectedCounting;
Int randomSeed;
bool multiplePrecision;
Int verboseCnf;
Int verboseSolving;

TimePoint toolStartPoint;

/* namespaces =============================================================== */

/* namespace util =========================================================== */

vector<Int> util::getSortedNums(const Set<Int>& nums) {
  vector<Int> v(nums.begin(), nums.end());
  sort(v.begin(), v.end());
  return v;
}

map<Int, string> util::getVarOrderHeuristics() {
  map<Int, string> m = CNF_VAR_ORDER_HEURISTICS;
  m.insert(JOIN_TREE_VAR_ORDER_HEURISTICS.begin(), JOIN_TREE_VAR_ORDER_HEURISTICS.end());
  return m;
}

string util::helpVarOrderHeuristic(const map<Int, string>& heuristics) {
  string s = ": ";
  for (auto it = heuristics.begin(); it != heuristics.end(); it++) {
    s += to_string(it->first) + "/" + it->second;
    if (next(it) != heuristics.end()) {
      s += ", ";
    }
  }
  return s + " (negatives for inverse orders); int";
}

string util::helpVerboseCnfProcessing() {
  return "verbose CNF processing: 0, 1, 2, 3; int";
}

string util::helpVerboseSolving() {
  return "verbose solving: 0, 1, 2; int";
}

TimePoint util::getTimePoint() {
  return std::chrono::steady_clock::now();
}

Float util::getDuration(TimePoint start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(getTimePoint() - start).count() / 1e3l;
}

vector<string> util::splitInputLine(const string& line) {
  std::istringstream inStringStream(line);
  vector<string> words;
  copy(istream_iterator<string>(inStringStream), istream_iterator<string>(), back_inserter(words));
  return words;
}

void util::printInputLine(const string& line, Int lineIndex) {
  cout << "c line " << right << setw(5) << lineIndex << ":" << (line.empty() ? "" : " " + line) << "\n";
}

void util::printRowKey(const string& key, size_t keyWidth) {
  string prefix = key;
  if (key != "s") {
    prefix = "c " + prefix;
  }

  keyWidth = max(keyWidth, prefix.size() + 1);
  cout << left << setw(keyWidth) << prefix;
}

/* classes for exceptions =================================================== */

/* class EmptyClauseException =============================================== */

EmptyClauseException::EmptyClauseException(Int lineIndex, const string& line) {
  cout << WARNING << "empty clause | line " << lineIndex << ": " << line << "\n";
}

/* class EmptyClauseException =============================================== */

UnsatSolverException::UnsatSolverException() {
  cout << WARNING << "unsatisfiable CNF, according to SAT solver\n";
}

/* classes for CNF formulas ================================================= */

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
    *this = Number(n.quotient);
  }
  else {
    *this = Number(n.fraction);
  }
}

Number::Number(const string& repr) {
  Int divPos = repr.find('/');
  if (multiplePrecision) {
    if (divPos != string::npos) { // repr is <int>/<int>
      *this = Number(mpq_class(repr));
    }
    else { // repr is <float>
      *this = Number(mpq_class(mpf_class(repr)));
    }
  }
  else {
    if (divPos != string::npos) { // repr is <int>/<int>
      Float numerator = stold(repr.substr(0, divPos));
      Float denominator = stold(repr.substr(divPos + 1));
      *this = Number(numerator / denominator);
    }
    else { // repr is <float>
      *this = Number(stold(repr));
    }
  }
}

Number Number::getAbsolute() const {
  if (multiplePrecision) {
    return Number(abs(quotient));
  }
  return Number(fabsl(fraction));
}

Float Number::getLog10() const {
  if (multiplePrecision) {
    mpf_t f; // C interface
    mpf_init(f);
    mpf_set_q(f, quotient.get_mpq_t());
    long int exponent;
    Float d = mpf_get_d_2exp(&exponent, f); // f == d * 2^exponent
    Float lgF = log10l(d) + exponent * log10l(2);
    mpf_clear(f);
    return lgF;
  }
  return log10l(fraction);
}

Float Number::getLogSumExp(const Number& n) const {
  assert(!multiplePrecision);
  if (fraction == -INF) {
    return n.fraction;
  }
  if (n.fraction == -INF) {
    return fraction;
  }
  Float m = max(fraction, n.fraction);
  return log10l(exp10l(fraction - m) + exp10l(n.fraction - m)) + m; // base-10 Cudd_addLogSumExp
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

bool Number::operator<(const Number& n) const {
  if (multiplePrecision) {
    return quotient < n.quotient;
  }
  return fraction < n.fraction;
}

bool Number::operator<=(const Number& n) const {
  return *this < n || *this == n;
}

bool Number::operator>(const Number& n) const {
  if (multiplePrecision) {
    return quotient > n.quotient;
  }
  return fraction > n.fraction;
}

bool Number::operator>=(const Number& n) const {
  return *this > n || *this == n;
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

/* class Graph ============================================================== */

Graph::Graph(const Set<Int>& vs) {
  for (Int v : vs) {
    vertices.insert(v);
    adjacencyMap[v] = Set<Int>();
  }
}

void Graph::addEdge(Int v1, Int v2) {
  adjacencyMap.at(v1).insert(v2);
  adjacencyMap.at(v2).insert(v1);
}

bool Graph::isNeighbor(Int v1, Int v2) const {
  return adjacencyMap.at(v1).contains(v2);
}

bool Graph::hasPath(Int from, Int to, Set<Int>& visitedVertices) const {
  if (from == to) {
    return true;
  }

  visitedVertices.insert(from);

  Set<Int> unvisitedNeighbors = util::getDiff(adjacencyMap.at(from), visitedVertices);

  for (Int v : unvisitedNeighbors) {
    if (hasPath(v, to, visitedVertices)) {
      return true;
    }
  }
  return false;
}

bool Graph::hasPath(Int from, Int to) const {
  Set<Int> visitedVertices;
  return hasPath(from, to, visitedVertices);
}

void Graph::removeVertex(Int v) {
  vertices.erase(v);

  adjacencyMap.erase(v); // edges from v

  for (pair<const Int, Set<Int>>& vertexAndNeighbors : adjacencyMap) {
    vertexAndNeighbors.second.erase(v); // edge to v
  }
}

void Graph::fillInEdges(Int v) {
  const Set<Int>& neighbors = adjacencyMap.at(v);
  for (auto neighbor1 = neighbors.begin(); neighbor1 != neighbors.end(); neighbor1++) {
    for (auto neighbor2 = next(neighbor1); neighbor2 != neighbors.end(); neighbor2++) {
      addEdge(*neighbor1, *neighbor2);
    }
  }
}

Int Graph::getFillInEdgeCount(Int v) const {
  Int edgeCount = 0;
  const Set<Int>& neighbors = adjacencyMap.at(v);
  for (auto neighbor1 = neighbors.begin(); neighbor1 != neighbors.end(); neighbor1++) {
    for (auto neighbor2 = next(neighbor1); neighbor2 != neighbors.end(); neighbor2++) {
      if (!isNeighbor(*neighbor1, *neighbor2)) {
        edgeCount++;
      }
    }
  }
  return edgeCount;
}

Int Graph::getMinFillVertex() const {
  Int vertex = MIN_INT;
  Int fillInEdgeCount = MAX_INT;

  for (Int v : vertices) {
    Int edgeCount = getFillInEdgeCount(v);
    if (edgeCount < fillInEdgeCount) {
      fillInEdgeCount = edgeCount;
      vertex = v;
    }
  }

  if (vertex == MIN_INT) {
    throw MyError("graph has no vertex");
  }

  return vertex;
}

/* class Label ============================================================== */

void Label::addNumber(Int i) {
  push_back(i);
  sort(begin(), end(), greater<Int>());
}

bool Label::hasSmallerLabel(const pair<Int, Label>& a, const pair <Int, Label>& b) {
  return a.second < b.second;
}

/* class Clause ============================================================= */

Clause::Clause(bool xorFlag) {
  this->xorFlag = xorFlag;
}

void Clause::insertLiteral(Int literal) {
  if (xorFlag && contains(literal)) {
    erase(literal);
  }
  else {
    insert(literal);
  }
}

void Clause::printClause() const {
  cout << (xorFlag ? " x" : "  ");
  for (auto it = begin(); it != end(); it++) {
    cout << " " << right << setw(5) << *it;
  }
  cout << "\n";
}

Set<Int> Clause::getClauseVars() const {
  Set<Int> vars;
  for (Int literal : *this) {
    vars.insert(abs(literal));
  }
  return vars;
}

/* class Cnf ================================================================ */

Set<Int> Cnf::getInnerVars() const {
  Set<Int> innerVars;
  for (Int var = 1; var <= declaredVarCount; var++) {
    if (!outerVars.contains(var)) {
      innerVars.insert(var);
    }
  }
  return innerVars;
}

Map<Int, Number> Cnf::getUnprunableWeights() const {
  Map<Int, Number> unprunableWeights;
  for (const auto& [literal, weight] : literalWeights) {
    if (weight > Number("1")) {
      unprunableWeights[literal] = weight;
    }
  }
  return unprunableWeights;
}

void Cnf::printLiteralWeight(Int literal, const Number& weight) {
  cout << "c  weight " << right << setw(5) << literal << ": " << weight << "\n";
}

void Cnf::printLiteralWeights() const {
  cout << "c literal weights:\n";
  for (Int var = 1; var <= declaredVarCount; var++) {
    printLiteralWeight(var, literalWeights.at(var));
    printLiteralWeight(-var, literalWeights.at(-var));
  }
}

void Cnf::printClauses() const {
  cout << "c CNF formula:\n";
  for (Int i = 0; i < clauses.size(); i++) {
    cout << "c  clause " << right << setw(5) << i + 1 << ":";
    clauses.at(i).printClause();
  }
}

void Cnf::addClause(const Clause& clause) {
  Int clauseIndex = clauses.size();
  clauses.push_back(clause);
  for (Int literal : clause) {
    Int var = abs(literal);
    auto it = varToClauses.find(var);
    if (it != varToClauses.end()) {
      it->second.insert(clauseIndex);
    }
    else {
      varToClauses[var] = {clauseIndex};
    }
  }
}

void Cnf::setApparentVars() {
  for (const pair<Int, Set<Int>>& kv : varToClauses) {
    apparentVars.insert(kv.first);
  }
}

Graph Cnf::getPrimalGraph() const {
  Graph graph(apparentVars);
  for (const Clause& clause : clauses) {
    for (auto literal1 = clause.begin(); literal1 != clause.end(); literal1++) {
      for (auto literal2 = next(literal1); literal2 != clause.end(); literal2++) {
        Int var1 = abs(*literal1);
        Int var2 = abs(*literal2);
        graph.addEdge(var1, var2);
      }
    }
  }
  return graph;
}

vector<Int> Cnf::getRandomVarOrder() const {
  vector<Int> varOrder(apparentVars.begin(), apparentVars.end());
  std::mt19937 generator;
  generator.seed(randomSeed);
  shuffle(varOrder.begin(), varOrder.end(), generator);
  return varOrder;
}

vector<Int> Cnf::getDeclarationVarOrder() const {
  vector<Int> varOrder;
  for (Int var = 1; var <= declaredVarCount; var++) {
    if (apparentVars.contains(var)) {
      varOrder.push_back(var);
    }
  }
  return varOrder;
}

vector<Int> Cnf::getMostClausesVarOrder() const {
  multimap<Int, Int, greater<Int>> m; // clause count |-> var
  for (const auto& [var, clauseIndices] : varToClauses) {
    m.insert({clauseIndices.size(), var});
  }

  vector<Int> varOrder;
  for (const auto& [clauseCount, var] : m) {
    varOrder.push_back(var);
  }
  return varOrder;
}

vector<Int> Cnf::getMinFillVarOrder() const {
  vector<Int> varOrder;

  Graph graph = getPrimalGraph();
  while (!graph.vertices.empty()) {
    Int vertex = graph.getMinFillVertex();
    graph.fillInEdges(vertex);
    graph.removeVertex(vertex);
    varOrder.push_back(vertex);
  }

  return varOrder;
}

vector<Int> Cnf::getMcsVarOrder() const {
  Graph graph = getPrimalGraph();

  auto startVertex = graph.vertices.begin();
  if (startVertex == graph.vertices.end()) {
    return vector<Int>();
  }

  Map<Int, Int> rankedNeighborCounts; // unranked vertex |-> number of ranked neighbors
  for (auto it = next(startVertex); it != graph.vertices.end(); it++) {
    rankedNeighborCounts[*it] = 0;
  }

  Int bestVertex = *startVertex;
  Int bestRankedNeighborCount = MIN_INT;

  vector<Int> varOrder;
  do {
    varOrder.push_back(bestVertex);

    rankedNeighborCounts.erase(bestVertex);

    for (Int n : graph.adjacencyMap.at(bestVertex)) {
      auto entry = rankedNeighborCounts.find(n);
      if (entry != rankedNeighborCounts.end()) {
        entry->second++;
      }
    }

    bestRankedNeighborCount = MIN_INT;
    for (pair<Int, Int> entry : rankedNeighborCounts) {
      if (entry.second > bestRankedNeighborCount) {
        bestRankedNeighborCount = entry.second;
        bestVertex = entry.first;
      }
    }
  }
  while (bestRankedNeighborCount != MIN_INT);

  return varOrder;
}

vector<Int> Cnf::getLexPVarOrder() const {
  Map<Int, Label> unnumberedVertices;
  for (Int vertex : apparentVars) {
    unnumberedVertices[vertex] = Label();
  }
  vector<Int> numberedVertices; // whose alpha numbers are decreasing
  Graph graph = getPrimalGraph();
  for (Int number = apparentVars.size(); number > 0; number--) {
    Int vertex = max_element(unnumberedVertices.begin(), unnumberedVertices.end(), Label::hasSmallerLabel)->first; // ignores label
    numberedVertices.push_back(vertex);
    unnumberedVertices.erase(vertex);
    for (Int neighbor : graph.adjacencyMap.at(vertex)) {
      auto unnumberedNeighborIt = unnumberedVertices.find(neighbor);
      if (unnumberedNeighborIt != unnumberedVertices.end()) {
        Int unnumberedNeighbor = unnumberedNeighborIt->first;
        unnumberedVertices.at(unnumberedNeighbor).addNumber(number);
      }
    }
  }
  return numberedVertices;
}

vector<Int> Cnf::getLexMVarOrder() const {
  Map<Int, Label> unnumberedVertices;
  for (Int vertex : apparentVars) {
    unnumberedVertices[vertex] = Label();
  }
  vector<Int> numberedVertices; // whose alpha numbers are decreasing
  Graph graph = getPrimalGraph();
  for (Int i = apparentVars.size(); i > 0; i--) {
    Int v = max_element(unnumberedVertices.begin(), unnumberedVertices.end(), Label::hasSmallerLabel)->first; // ignores label
    numberedVertices.push_back(v);
    unnumberedVertices.erase(v);

    /* updates numberedVertices: */
    Graph subgraph = getPrimalGraph(); // will only contain v, w, and unnumbered vertices whose labels are less than w's label
    for (pair<const Int, Label>& wAndLabel : unnumberedVertices) {
      Int w = wAndLabel.first;
      Label& wLabel = wAndLabel.second;

      /* removes numbered vertices except v: */
      for (Int numberedVertex : numberedVertices) {
        if (numberedVertex != v) {
          subgraph.removeVertex(numberedVertex);
        }
      }

      /* removes each non-w unnumbered vertex whose label is at least w's labels */
      for (const pair<Int, Label>& kv : unnumberedVertices) {
        Int unnumberedVertex = kv.first;
        const Label& label = kv.second;
        if (unnumberedVertex != w && label >= wLabel) {
          subgraph.removeVertex(unnumberedVertex);
        }
      }

      if (subgraph.hasPath(v, w)) {
        wLabel.addNumber(i);
      }
    }
  }
  return numberedVertices;
}

vector<Int> Cnf::getCnfVarOrder(Int cnfVarOrderHeuristic) const {
  vector<Int> varOrder;
  switch (abs(cnfVarOrderHeuristic)) {
    case RANDOM_HEURISTIC:
      varOrder = getRandomVarOrder();
      break;
    case DECLARATION_HEURISTIC:
      varOrder = getDeclarationVarOrder();
      break;
    case MOST_CLAUSES_HEURISTIC:
      varOrder = getMostClausesVarOrder();
      break;
    case MIN_FILL_HEURISTIC:
      varOrder = getMinFillVarOrder();
      break;
    case MCS_HEURISTIC:
      varOrder = getMcsVarOrder();
      break;
    case LEX_P_HEURISTIC:
      varOrder = getLexPVarOrder();
      break;
    default:
      assert(abs(cnfVarOrderHeuristic) == LEX_M_HEURISTIC);
      varOrder = getLexMVarOrder();
  }

  if (cnfVarOrderHeuristic < 0) {
    reverse(varOrder.begin(), varOrder.end());
  }

  return varOrder;
}

bool Cnf::isMc21ShowLine(const vector<string> &words) const {
  return words.size() >= 4 && words.front() == "c" && words.at(1) == "p" && words.at(2) == "show";
}

bool Cnf::isMc21WeightLine(const vector<string> &words) const {
  bool b = words.size() >= 3 && words.front() == "c" && words.at(1) == "p" && words.at(2) == "weight";
  switch (words.size()) {
    case 5:
      return b;
    case 6:
      return b && words.back() == "0";
    default:
      return false;
  }
}

void Cnf::completeImplicitLiteralWeight(Int literal) {
  if (!literalWeights.contains(literal) && literalWeights.contains(-literal)){
    literalWeights[literal] = Number("1") - literalWeights.at(-literal);
    if (literalWeights[literal] < Number()) {
      throw MyError("literal ", literal, " has implicit weight ", literalWeights[literal], " < 0");
    }
  }
}

void Cnf::completeLiteralWeights() {
  if (weightedCounting) {
    for (Int var = 1; var <= declaredVarCount; var++) {
      completeImplicitLiteralWeight(var);
      completeImplicitLiteralWeight(-var);
      if (!literalWeights.contains(var) && !literalWeights.contains(-var)) {
        literalWeights[var] = Number("1");
        literalWeights[-var] = Number("1");
      }
    }
  }
  else {
    for (Int var = 1; var <= declaredVarCount; var++) {
      literalWeights[var] = Number("1");
      literalWeights[-var] = Number("1");
    }
  }
}

void Cnf::printStats() const {
  Float clauseSizeSum = 0;
  Int clauseSizeMax = MIN_INT;
  Int clauseSizeMin = MAX_INT;

  for (const Clause& clause : clauses) {
    Int clauseSize = clause.size();
    clauseSizeSum += clauseSize;
    clauseSizeMax = max(clauseSizeMax, clauseSize);
    clauseSizeMin = min(clauseSizeMin, clauseSize);
  }

  util::printRow("clauseSizeMean", clauseSizeSum / clauses.size());
  util::printRow("clauseSizeMax", clauseSizeMax);
  util::printRow("clauseSizeMin", clauseSizeMin);
}

void Cnf::readCnfFile(const string& filePath) {
  cout << "c processing CNF formula...\n";

  std::ifstream inputFileStream(filePath);
  if (!inputFileStream.is_open()) {
    throw MyError("unable to open file '", filePath, "'");
  }

  Int declaredClauseCount = MIN_INT;

  Int lineIndex = 0;
  Int problemLineIndex = MIN_INT;

  string line;
  while (getline(inputFileStream, line)) {
    lineIndex++;
    std::istringstream inStringStream(line);

    if (verboseCnf >= 3) {
      util::printInputLine(line, lineIndex);
    }

    vector<string> words = util::splitInputLine(line);
    if (words.empty()) {
      continue;
    }
    string& frontWord = words.front();
    if (Set<string>{"s", "INDETERMINATE"}.contains(frontWord)) { // preprocessor pmc
      throw MyError("unexpected output from preprocessor pmc | line ", lineIndex, ": ", line);
    }
    else if (frontWord == "p") { // problem line
      if (problemLineIndex != MIN_INT) {
        throw MyError("multiple problem lines: ", problemLineIndex, " and ", lineIndex);
      }

      problemLineIndex = lineIndex;

      if (words.size() != 4) {
        throw MyError("problem line ", lineIndex, " has ", words.size(), " words (should be 4)");
      }

      declaredVarCount = stoll(words.at(2));
      declaredClauseCount = stoll(words.at(3));
    }
    else if (frontWord == "c") { // possibly show or weight line
      if (projectedCounting && isMc21ShowLine(words)) {
        if (problemLineIndex == MIN_INT) {
          throw MyError("no problem line before outer vars | line ", lineIndex, ": ", line);
        }

        for (Int i = 3; i < words.size(); i++) {
          Int num = stoll(words.at(i));
          if (num == 0) {
            if (i != words.size() - 1) {
              throw MyError("outer vars terminated prematurely by '0' | line ", lineIndex);
            }
          }
          else if (num < 0 || num > declaredVarCount) {
            throw MyError("var '", num, "' inconsistent with declared var count '", declaredVarCount, "' | line ", lineIndex);
          }
          else {
            outerVars.insert(num);
          }
        }
      }
      else if (weightedCounting && isMc21WeightLine(words)) {
        if (problemLineIndex == MIN_INT) {
          throw MyError("no problem line before literal weight | line ", lineIndex, ": ", line);
        }

        Int literal = stoll(words.at(3));
        assert(literal != 0);

        if (abs(literal) > declaredVarCount) {
          throw MyError("literal '", literal, "' inconsistent with declared var count '", declaredVarCount, "' | line ", lineIndex);
        }

        Number weight(words.at(4));
        if (weight < Number()) {
          throw MyError("literal weight must be non-negative | line ", lineIndex);
        }
        literalWeights[literal] = weight;
      }
    }
    else if (!frontWord.starts_with("c")) { // clause line
      if (problemLineIndex == MIN_INT) {
        throw MyError("no problem line before clause | line ", lineIndex);
      }

      bool xorFlag = false;
      if (frontWord.starts_with("x")) {
        xorFlag = true;
        xorClauseCount++;

        if (frontWord == "x") {
          words.erase(words.begin());
        }
        else {
          frontWord.erase(frontWord.begin());
        }
      }
      Clause clause(xorFlag);

      for (Int i = 0; i < words.size(); i++) {
        Int num = stoll(words.at(i));

        if (abs(num) > declaredVarCount) {
          throw MyError("literal '", num, "' inconsistent with declared var count '", declaredVarCount, "' | line ", lineIndex);
        }

        if (num == 0) {
          if (i != words.size() - 1) {
            throw MyError("clause terminated prematurely by '0' | line ", lineIndex);
          }

          if (clause.empty()) {
            throw EmptyClauseException(lineIndex, line);
          }

          addClause(clause);
        }
        else { // literal
          if (i == words.size() - 1) {
            throw MyError("missing end-of-clause indicator '0' | line ", lineIndex);
          }
          clause.insertLiteral(num);
        }
      }
    }
  }

  if (problemLineIndex == MIN_INT) {
    throw MyError("no problem line before CNF file ends on line ", lineIndex);
  }

  setApparentVars();

  if (!projectedCounting) {
    for (Int var = 1; var <= declaredVarCount; var++) {
      outerVars.insert(var);
    }
  }

  completeLiteralWeights();

  if (verboseCnf >= 1) {
    util::printRow("declaredVarCount", declaredVarCount);
    util::printRow("apparentVarCount", apparentVars.size());

    util::printRow("declaredClauseCount", declaredClauseCount);
    util::printRow("apparentClauseCount", clauses.size());
    util::printRow("xorClauseCount", xorClauseCount);

    printStats();

    if (verboseCnf >= 2) {
      if (projectedCounting) {
        cout << "c outer vars: { ";
        for (Int var : util::getSortedNums(outerVars)) {
          cout << var << " ";
        }
        cout << "}\n";
      }

      if (weightedCounting) {
        printLiteralWeights();
      }

      printClauses();
    }
  }

  cout << "\n";
}

Cnf::Cnf() {}

/* classes for join trees =================================================== */

/* class Assignment ========================================================= */

Assignment::Assignment() {}

Assignment::Assignment(Int var, bool val) {
  insert({var, val});
}

Assignment::Assignment(const string& bitString) {
  for (Int i = 0; i < bitString.size(); i++) {
    char bit = bitString.at(i);
    assert(bit == '0' or bit == '1');
    insert({i + 1, bit == '1'});
  }
}

bool Assignment::getValue(Int var) const {
  auto it = find(var);
  return (it != end()) ? it->second : true;
}

void Assignment::printAssignment() const {
  for (auto it = begin(); it != end(); it++) {
    cout << right << setw(5) << (it->second ? it->first : -it->first);
    if (next(it) != end()) {
      cout << " ";
    }
  }
}

vector<Assignment> Assignment::getExtendedAssignments(const vector<Assignment>& assignments, Int var) {
  vector<Assignment> extendedAssignments;
  if (assignments.empty()) {
    extendedAssignments.push_back(Assignment(var, false));
    extendedAssignments.push_back(Assignment(var, true));
  }
  else {
    for (Assignment assignment : assignments) {
      assignment[var] = false;
      extendedAssignments.push_back(assignment);
      assignment[var] = true;
      extendedAssignments.push_back(assignment);
    }
  }
  return extendedAssignments;
}

/* class JoinNode =========================================================== */

Int JoinNode::nodeCount;
Int JoinNode::terminalCount;
Set<Int> JoinNode::nonterminalIndices;

Int JoinNode::backupNodeCount;
Int JoinNode::backupTerminalCount;
Set<Int> JoinNode::backupNonterminalIndices;

Cnf JoinNode::cnf;

void JoinNode::resetStaticFields() {
  backupNodeCount = nodeCount;
  backupTerminalCount = terminalCount;
  backupNonterminalIndices = nonterminalIndices;

  nodeCount = 0;
  terminalCount = 0;
  nonterminalIndices.clear();
}

void JoinNode::restoreStaticFields() {
  nodeCount = backupNodeCount;
  terminalCount = backupTerminalCount;
  nonterminalIndices = backupNonterminalIndices;
}

Set<Int> JoinNode::getPostProjectionVars() const {
  return util::getDiff(preProjectionVars, projectionVars);
}

Int JoinNode::chooseClusterIndex(Int clusterIndex, const vector<Set<Int>>& projectableVarSets, string clusteringHeuristic) {
  if (clusterIndex < 0 || clusterIndex >= projectableVarSets.size()) {
    throw MyError("clusterIndex == ", clusterIndex, " whereas projectableVarSets.size() == ", projectableVarSets.size());
  }

  Set<Int> projectableVars = util::getUnion(projectableVarSets); // Z = Z_1 \cup .. \cup Z_m
  Set<Int> postProjectionVars = getPostProjectionVars(); // of this node
  if (util::isDisjoint(projectableVars, postProjectionVars)) {
    return projectableVarSets.size(); // special cluster
  }

  if (clusteringHeuristic == BUCKET_ELIM_LIST || clusteringHeuristic == BOUQUET_METHOD_LIST) {
    return clusterIndex + 1;
  }
  for (Int target = clusterIndex + 1; target < projectableVarSets.size(); target++) {
    if (!util::isDisjoint(postProjectionVars, projectableVarSets.at(target))) {
      return target;
    }
  }
  return projectableVarSets.size();
}

Int JoinNode::getNodeRank(const vector<Int>& restrictedVarOrder, string clusteringHeuristic) {
  Set<Int> postProjectionVars = getPostProjectionVars();

  if (clusteringHeuristic == BUCKET_ELIM_LIST || clusteringHeuristic == BUCKET_ELIM_TREE) { // min var rank
    Int rank = MAX_INT;
    for (Int varRank = 0; varRank < restrictedVarOrder.size(); varRank++) {
      if (postProjectionVars.contains(restrictedVarOrder.at(varRank))) {
        rank = min(rank, varRank);
      }
    }
    return (rank == MAX_INT) ? restrictedVarOrder.size() : rank;
  }

  Int rank = MIN_INT;
  for (Int varRank = 0; varRank < restrictedVarOrder.size(); varRank++) {
    if (postProjectionVars.contains(restrictedVarOrder.at(varRank))) {
      rank = max(rank, varRank);
    }
  }
  return (rank == MIN_INT) ? restrictedVarOrder.size() : rank;
}

bool JoinNode::isTerminal() const {
  return nodeIndex < terminalCount;
}

/* class JoinTerminal ======================================================= */

Int JoinTerminal::getWidth(const Assignment& assignment) const {
  return util::getDiff(preProjectionVars, assignment).size();
}

void JoinTerminal::updateVarSizes(Map<Int, size_t>& varSizes) const {
  Set<Int> vars = cnf.clauses.at(nodeIndex).getClauseVars();
  for (Int var : vars) {
    varSizes[var] = max(varSizes[var], vars.size());
  }
}

JoinTerminal::JoinTerminal() {
  nodeIndex = terminalCount;
  terminalCount++;
  nodeCount++;

  preProjectionVars = cnf.clauses.at(nodeIndex).getClauseVars();
}

/* class JoinNonterminal ===================================================== */

void JoinNonterminal::printNode(const string& startWord) const {
  cout << startWord << nodeIndex + 1 << " ";

  for (const JoinNode* child : children) {
    cout << child->nodeIndex + 1 << " ";
  }

  cout << ELIM_VARS_WORD;
  for (Int var : projectionVars) {
    cout << " " << var;
  }

  cout << "\n";
}

void JoinNonterminal::printSubtree(const string& startWord) const {
  for (const JoinNode* child : children) {
    if (!child->isTerminal()) {
      static_cast<const JoinNonterminal*>(child)->printSubtree(startWord);
    }
  }
  printNode(startWord);
}

Int JoinNonterminal::getWidth(const Assignment& assignment) const {
  Int width = util::getDiff(preProjectionVars, assignment).size();
  for (JoinNode* child : children) {
    width = max(width, child->getWidth(assignment));
  }
  return width;
}

void JoinNonterminal::updateVarSizes(Map<Int, size_t>& varSizes) const {
  for (Int var : preProjectionVars) {
    varSizes[var] = max(varSizes[var], preProjectionVars.size());
  }
  for (JoinNode* child : children) {
    child->updateVarSizes(varSizes);
  }
}

vector<Int> JoinNonterminal::getBiggestNodeVarOrder() const {
  Map<Int, size_t> varSizes; // var x |-> size of biggest node containing x
  for (Int var : cnf.apparentVars) {
    varSizes[var] = 0;
  }

  updateVarSizes(varSizes);

  multimap<size_t, Int, greater<size_t>> sizedVars = util::flipMap(varSizes); // size |-> var

  size_t prevSize = 0;

  if (verboseSolving >= 2) {
    cout << DASH_LINE;
  }

  vector<Int> varOrder;
  for (const auto& [varSize, var] : sizedVars) {
    varOrder.push_back(var);

    if (verboseSolving >= 2) {
      if (prevSize == varSize) {
        cout << " " << var;
      }
      else {
        if (prevSize > 0) {
          cout << "\n";
        }
        prevSize = varSize;
        cout << "c vars in nodes of size " << right << setw(5) << varSize << ": " << var;
      }
    }
  }

  if (verboseSolving >= 2) {
    cout << "\n" << DASH_LINE;
  }

  return varOrder;
}

vector<Int> JoinNonterminal::getHighestNodeVarOrder() const {
  vector<Int> varOrder;
  std::queue<const JoinNonterminal*> q;
  q.push(this);
  while (!q.empty()) {
    const JoinNonterminal* n = q.front();
    q.pop();
    for (Int var : n->projectionVars) {
      varOrder.push_back(var);
    }
    for (const JoinNode* child : n->children) {
      if (!child->isTerminal()) {
        q.push(static_cast<const JoinNonterminal*>(child));
      }
    }
  }
  return varOrder;
}

vector<Int> JoinNonterminal::getVarOrder(Int varOrderHeuristic) const {
  if (CNF_VAR_ORDER_HEURISTICS.contains(abs(varOrderHeuristic))) {
    return cnf.getCnfVarOrder(varOrderHeuristic);
  }

  vector<Int> varOrder;
  if (abs(varOrderHeuristic) == BIGGEST_NODE_HEURISTIC) {
    varOrder = getBiggestNodeVarOrder();
  }
  else {
    assert(abs(varOrderHeuristic) == HIGHEST_NODE_HEURISTIC);
    varOrder = getHighestNodeVarOrder();
  }

  if (varOrderHeuristic < 0) {
    reverse(varOrder.begin(), varOrder.end());
  }

  return varOrder;
}

vector<Assignment> JoinNonterminal::getOuterAssignments(Int varOrderHeuristic, Int sliceVarCount) const {
  if (sliceVarCount <= 0) {
    return {Assignment()};
  }

  TimePoint sliceVarOrderStartPoint = util::getTimePoint();
  vector<Int> varOrder = getVarOrder(varOrderHeuristic);
  if (verboseSolving >= 1) {
    util::printRow("sliceVarSeconds", util::getDuration(sliceVarOrderStartPoint));
  }

  TimePoint assignmentsStartPoint = util::getTimePoint();
  vector<Assignment> assignments;

  if (verboseSolving >= 2) {
    cout << "c slice var order heuristic: {";
  }

  for (Int i = 0, assignedVars = 0; i < varOrder.size() && assignedVars < sliceVarCount; i++) {
    Int var = varOrder.at(i);
    if (cnf.outerVars.contains(var)) {
      assignments = Assignment::getExtendedAssignments(assignments, var);
      assignedVars++;
      if (verboseSolving >= 2) {
        cout << " " << var;
      }
    }
  }

  if (verboseSolving >= 2) {
    cout << " }\n";
  }

  if (verboseSolving >= 1) {
    util::printRow("sliceAssignmentsSeconds", util::getDuration(assignmentsStartPoint));
  }

  return assignments;
}

JoinNonterminal::JoinNonterminal(const vector<JoinNode*>& children, const Set<Int>& projectionVars, Int requestedNodeIndex) {
  this->children = children;
  this->projectionVars = projectionVars;

  if (requestedNodeIndex == MIN_INT) {
    requestedNodeIndex = nodeCount;
  }
  else if (requestedNodeIndex < terminalCount) {
    throw MyError("requestedNodeIndex == ", requestedNodeIndex, " < ", terminalCount, " == terminalCount");
  }
  else if (nonterminalIndices.contains(requestedNodeIndex)) {
    throw MyError("requestedNodeIndex ", requestedNodeIndex, " already taken");
  }

  nodeIndex = requestedNodeIndex;
  nonterminalIndices.insert(nodeIndex);
  nodeCount++;

  for (JoinNode* child : children) {
    util::unionize(preProjectionVars, child->getPostProjectionVars());
  }
}

/* global functions ========================================================= */

ostream& operator<<(ostream& stream, const Number& n) {
  if (multiplePrecision) {
    stream << n.quotient;
  }
  else {
    stream << n.fraction;
  }

  return stream;
}
