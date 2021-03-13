/* inclusions =============================================================== */

#include "dmc.hh"

/* global vars ============================================================== */

string planningStrategy;
Int threadCount;
string ddPackage;
Int maxMegabytes;
string joinPriority;

/* classes for processing jointrees ========================================= */

/* class JoinTree =========================================================== */

JoinNode* JoinTree::getJoinNode(Int nodeIndex) const {
  if (joinTerminals.contains(nodeIndex)) {
    return joinTerminals.at(nodeIndex);
  }
  return joinNonterminals.at(nodeIndex);
}

JoinNonterminal* JoinTree::getJoinRoot() const {
  return joinNonterminals.at(declaredNodeCount - 1);
}

void JoinTree::printTree() const {
  cout << "c " << PROBLEM_WORD << " " << JT_WORD << " " << declaredVarCount << " " << declaredClauseCount << " " << declaredNodeCount << "\n";
  getJoinRoot()->printSubtree("c ");
}

JoinTree::JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount) {
  this->declaredVarCount = declaredVarCount;
  this->declaredClauseCount = declaredClauseCount;
  this->declaredNodeCount = declaredNodeCount;
}

/* class JoinTreeProcessor ================================================== */

Int JoinTreeProcessor::plannerPid = MIN_INT;

void JoinTreeProcessor::killPlanner() {
  if (plannerPid == MIN_INT) {
    cout << WARNING << "found no PID for planner process\n";
  }
  else if (kill(plannerPid, SIGKILL) == 0) {
    cout << "c killed planner process with PID " << plannerPid << "\n";
  }
  else {
    cout << WARNING << "failed to kill planner process with PID " << plannerPid << "\n";
  }
}

const JoinNonterminal* JoinTreeProcessor::getJoinTreeRoot() const {
  return joinTree->getJoinRoot();
}

JoinTreeProcessor::JoinTreeProcessor() {
  util::setSignalHandler();
}

/* class JoinTreeParser ===================================================== */

void JoinTreeParser::finishParsingJoinTree() {
  if (joinTree == nullptr) {
    throw MyError("no jointree before line ", lineIndex);
  }

  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;
  if (nonterminalCount < expectedNonterminalCount) {
    throw MyError("missing internal nodes (", nonterminalCount, " found, ", expectedNonterminalCount, " expected) before jointree ends on line ", lineIndex);
  }

  cout << "c processed jointree ending on line " << lineIndex << "\n";
  util::printRow("jointreeWidth", joinTree->getJoinRoot()->getWidth());
  util::printRow("jointreeSeconds", joinTree->plannerSeconds);

  if (verbosityLevel > 1) {
    cout << THIN_LINE;
    joinTree->printTree();
    cout << THIN_LINE;
  }
}

void JoinTreeParser::parseInputStream() {
  string line;
  while (getline(std::cin, line)) {
    lineIndex++;

    if (verbosityLevel > 2) {
      util::printInputLine(lineIndex, line);
    }

    vector<string> words = util::splitWords(line);
    if (words.empty() || words.front() == "{'':") { // SlurmQueen `echo` line
      continue;
    }
    else if (words.front() == PROBLEM_WORD) {
      if (problemLineIndex != MIN_INT) {
        throw MyError("multiple problem lines: ", problemLineIndex, " and ", lineIndex);
      }
      problemLineIndex = lineIndex;

      if (words.size() != 5) {
        throw MyError("problem line ", lineIndex, " has ", words.size(), " words (should be 5)");
      }
      string jtWord = words.at(1);
      if (jtWord != JT_WORD) {
        throw MyError("expected '", JT_WORD, "', found '", jtWord, "' | line ", lineIndex);
      }
      Int declaredVarCount = stoll(words.at(2));
      Int declaredClauseCount = stoll(words.at(3));
      Int declaredNodeCount = stoll(words.at(4));
      joinTree = new JoinTree(declaredVarCount, declaredClauseCount, declaredNodeCount);
      for (Int terminalIndex = 0; terminalIndex < declaredClauseCount; terminalIndex++) {
        joinTree->joinTerminals[terminalIndex] = new JoinTerminal();
      }
    }
    else if (words.front() == "c") {
      if (words.size() == 3) {
        string key = words.at(1);
        string val = words.at(2);
        if (key == "pid") {
          plannerPid = stoll(val);
        }
        else if (key == "seconds") {
          if (joinTree != nullptr) {
            joinTree->plannerSeconds = stold(val);
            break;
          }
        }
      }
    }
    else { // internal-node line
      if (problemLineIndex == MIN_INT) {
        throw MyError("no problem line before internal node | line ", lineIndex);
      }

      Int parentIndex = stoll(words.front()) - 1; // 0-indexing
      if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
        throw MyError("wrong internal-node index | line ", lineIndex);
      }

      vector<JoinNode*> children;
      Set<Int> projectionVars;
      bool parsingElimVars = false;
      for (Int i = 1; i < words.size(); i++) {
        string word = words.at(i);
        if (word == VAR_ELIM_WORD) {
          parsingElimVars = true;
        }
        else {
          Int num = stoll(word);
          if (parsingElimVars) {
            if (num <= 0 || num > joinTree->declaredVarCount) {
              throw MyError("var '", num, "' inconsistent with declared var count '", joinTree->declaredVarCount, "' | line ", lineIndex);
            }
            projectionVars.insert(num);
          }
          else {
            Int childIndex = num - 1; // 0-indexing
            if (childIndex < 0 || childIndex >= parentIndex) {
              throw MyError("child '", word, "' wrong | line ", lineIndex);
            }
            children.push_back(joinTree->getJoinNode(childIndex));
          }
        }
      }
      joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectionVars, parentIndex);
    }
  }
}

JoinTreeParser::JoinTreeParser() {
  cout << "\n";
  cout << "c procressing jointree...\n";

  cout << THICK_LINE;
  cout << "c getting jointree from STDIN... (end input with 'Enter' then 'Ctrl d')\n";

  parseInputStream();
  finishParsingJoinTree();
  if (plannerPid != MIN_INT) {
    killPlanner();
  }

  cout << "c getting jointree from STDIN: done\n";
  cout << THICK_LINE;

  if (verbosityLevel > 0) {
    util::printRow("declaredNodeCount", joinTree->declaredNodeCount);
  }
}

/* class JoinTreeReader ===================================================== */

void JoinTreeReader::handleSigalrm(int signal) {
  assert(signal == SIGALRM);
  cout << "c received SIGALRM after " << util::getSeconds(startTime) << "s\n";
  killPlanner();
}

bool JoinTreeReader::hasDisarmedTimer() {
  struct itimerval curr_value;
  getitimer(ITIMER_REAL, addressof(curr_value));
  return curr_value.it_value.tv_sec == 0 && curr_value.it_value.tv_usec == 0 && curr_value.it_interval.tv_sec == 0 && curr_value.it_interval.tv_usec == 0;
}

void JoinTreeReader::setTimer(Float seconds) {
  assert(seconds >= 0);
  Int secs = seconds;
  Int usecs = (seconds - secs) * 1000000;
  struct itimerval new_value;
  new_value.it_value.tv_sec = secs;
  new_value.it_value.tv_usec = usecs;
  new_value.it_interval.tv_sec = 0;
  new_value.it_interval.tv_usec = 0;
  setitimer(ITIMER_REAL, addressof(new_value), nullptr);
}

void JoinTreeReader::disarmTimer() {
  setTimer(0);
  cout << "c disarmed timer\n";
}

void JoinTreeReader::armTimer(Float seconds) {
  assert(seconds > 0);
  signal(SIGALRM, handleSigalrm);
  setTimer(seconds);
  cout << "c set timer for " << seconds << "s\n";
}

void JoinTreeReader::finishReadingJoinTree() {
  if (joinTree == nullptr) {
    throw MyError("no jointree before line ", lineIndex);
  }

  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;
  if (nonterminalCount < expectedNonterminalCount) {
    cout << WARNING << "missing internal nodes (" << nonterminalCount << " found, " << expectedNonterminalCount << " expected) before current jointree ends on line " << lineIndex << "\n";
    if (joinTreeEndLineIndex == MIN_INT) {
      throw MyError("no backup jointree");
    }
    cout << WARNING << "restoring backup jointree ending on line " << joinTreeEndLineIndex << "\n";
    if (verbosityLevel > 0) {
      cout << THIN_LINE;
      cout << "c restored backup jointree:\n";
      backupJoinTree->printTree();
      cout << THIN_LINE;
    }
    joinTree = backupJoinTree;
    JoinNode::restoreStaticFields();
  }
  else {
    cout << "c processed jointree ending on line " << lineIndex << "\n";
    util::printRow("jointreeWidth", joinTree->getJoinRoot()->getWidth());
    util::printRow("jointreeSeconds", joinTree->plannerSeconds);
    if (verbosityLevel > 1) {
      cout << THIN_LINE;
      joinTree->printTree();
      cout << THIN_LINE;
    }
  }
  joinTreeEndLineIndex = lineIndex;
  problemLineIndex = MIN_INT;
}

void JoinTreeReader::readInputStream() {
  string line;
  while (getline(std::cin, line) && !hasDisarmedTimer()) {
    lineIndex++;

    if (verbosityLevel > 2) {
      util::printInputLine(lineIndex, line);
    }

    vector<string> words = util::splitWords(line);
    if (words.empty() || words.front() == "{'':" || words.front() == "=") {
      continue;
    }
    else if (words.front() == PROBLEM_WORD) {
      if (problemLineIndex != MIN_INT) {
        throw MyError("multiple problem lines: ", problemLineIndex, " and ", lineIndex);
      }
      problemLineIndex = lineIndex;

      if (words.size() != 5) {
        throw MyError("problem line ", lineIndex, " has ", words.size(), " words (should be 5)");
      }
      string jtWord = words.at(1);
      if (jtWord != JT_WORD) {
        throw MyError("expected '", JT_WORD, "', found '", jtWord, "' | line ", lineIndex);
      }

      Int declaredVarCount = stoll(words.at(2));
      Int declaredClauseCount = stoll(words.at(3));
      Int declaredNodeCount = stoll(words.at(4));
      backupJoinTree = joinTree;
      joinTree = new JoinTree(declaredVarCount, declaredClauseCount, declaredNodeCount);
      JoinNode::resetStaticFields();
      for (Int terminalIndex = 0; terminalIndex < declaredClauseCount; terminalIndex++) {
        joinTree->joinTerminals[terminalIndex] = new JoinTerminal();
      }
    }
    else if (words.front() == "c") {
      if (words.size() == 3) {
        string key = words.at(1);
        string val = words.at(2);
        if (key == "pid") {
          plannerPid = stoll(val);
        }
        else if (key == "seconds") {
          if (joinTree != nullptr) {
            joinTree->plannerSeconds = stold(val);
            finishReadingJoinTree();
          }
        }
      }
    }
    else { // internal-node line
      if (problemLineIndex == MIN_INT) {
        string message = "no problem line before internal node | line " + to_string(lineIndex);
        if (joinTreeEndLineIndex != MIN_INT) {
          message += " (previous jointree ends on line " + to_string(joinTreeEndLineIndex) + ")";
        }
        throw MyError(message);
      }
      Int parentIndex = stoll(words.front()) - 1; // 0-indexing
      if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
        throw MyError("wrong internal-node index | line ", lineIndex);
      }
      vector<JoinNode*> children;
      Set<Int> projectionVars;
      bool readingElimVars = false;
      for (Int i = 1; i < words.size(); i++) {
        string word = words.at(i);
        if (word == VAR_ELIM_WORD) {
          readingElimVars = true;
        }
        else {
          Int num = stoll(word);
          if (readingElimVars) {
            Int declaredVarCount = joinTree->declaredVarCount;
            if (num <= 0 || num > declaredVarCount) {
              throw MyError("var '", num, "' inconsistent with declared var count '", declaredVarCount, "' | line ", lineIndex);
            }
            projectionVars.insert(num);
          }
          else {
            Int childIndex = num - 1; // 0-indexing
            if (childIndex < 0 || childIndex >= parentIndex) {
              throw MyError("child '", word, "' wrong | line ", lineIndex);
            }
            children.push_back(joinTree->getJoinNode(childIndex));
          }
        }
      }
      joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectionVars, parentIndex);
    }
  }
  if (!hasDisarmedTimer()) {
    disarmTimer();
  }
}

JoinTreeReader::JoinTreeReader(Float jtWaitSeconds) {
  cout << "\n";
  cout << "c procressing jointree...\n";
  armTimer(jtWaitSeconds);

  cout << THICK_LINE;
  cout << "c getting jointree from STDIN with " << jtWaitSeconds << "s timer... (end input with 'Enter' then 'Ctrl d')\n";
  readInputStream();
  cout << "c getting jointree from STDIN: done\n";
  cout << THICK_LINE;

  if (verbosityLevel > 0) {
    util::printRow("declaredNodeCount", joinTree->declaredNodeCount);
  }
}

/* classes for decision diagrams ============================================ */

/* class Dd ================================================================= */

Dd::Dd(const ADD& cuadd) {
  assert(ddPackage == CUDD);
  this->cuadd = cuadd;
}

Dd::Dd(const Mtbdd& mtbdd) {
  assert(ddPackage == SYLVAN);
  this->mtbdd = mtbdd;
}

Dd::Dd(const Dd& dd) {
  if (ddPackage == CUDD) {
    this->cuadd = dd.cuadd;
  }
  else {
    this->mtbdd = dd.mtbdd;
  }
}

const Cudd* Dd::newMgr(Int maxMegs) {
  assert(ddPackage == CUDD);
  Cudd* mgr = new Cudd();
  mgr->SetMaxMemory(maxMegs * 1024 * 1024);
  return mgr;
}

Dd Dd::getConstDd(const Number& n, const Cudd* mgr) {
  if (ddPackage == CUDD) {
    return logCounting ? Dd(mgr->constant(n.getLn())) : Dd(mgr->constant(n.fraction));
  }
  if (multiplePrecision) {
    mpq_t q; // C interface
    mpq_init(q);
    mpq_set(q, n.quotient.get_mpq_t());
    Dd dd(Mtbdd(mtbdd_gmp(q)));
    mpq_clear(q);
    return dd;
  }
  return Dd(Mtbdd::doubleTerminal(n.fraction));
}

Dd Dd::getZeroDd(const Cudd* mgr) {
  return getConstDd(Number(), mgr);
}

Dd Dd::getOneDd(const Cudd* mgr) {
  return getConstDd(Number("1"), mgr);
}

Dd Dd::getVarDd(Int ddVar, bool val, const Cudd* mgr) {
  if (ddPackage == CUDD) {
    if (logCounting) {
      return Dd(mgr->addLogVar(ddVar, val));
    }
    return val ? Dd(mgr->addVar(ddVar)) : Dd((mgr->addVar(ddVar)).Cmpl());
  }
  if (val) {
    return Dd(mtbdd_makenode(ddVar, getZeroDd(mgr).mtbdd.GetMTBDD(), getOneDd(mgr).mtbdd.GetMTBDD())); // (var, lo, hi)
  }
  return Dd(mtbdd_makenode(ddVar, getOneDd(mgr).mtbdd.GetMTBDD(), getZeroDd(mgr).mtbdd.GetMTBDD()));
}

Int Dd::countNodes() const {
  if (ddPackage == CUDD) {
    return cuadd.nodeCount();
  }
  return mtbdd.NodeCount();
}

bool Dd::operator<(const Dd& rightDd) const {
  if (joinPriority == SMALLEST_PAIR) { // top = rightmost = smallest
    return countNodes() > rightDd.countNodes();
  }
  return countNodes() < rightDd.countNodes();
}

Number Dd::extractConst() const {
  if (ddPackage == CUDD) {
    ADD minTerminal = cuadd.FindMin();
    assert(minTerminal == cuadd.FindMax());
    return Number(cuddV(minTerminal.getNode()));
  }
  assert(mtbdd.isLeaf());
  if (multiplePrecision) {
    return Number(mpq_class((mpq_ptr)mtbdd_getvalue(mtbdd.GetMTBDD())));
  }
  return Number(mtbdd_getdouble(mtbdd.GetMTBDD()));
}

Dd Dd::getComposition(Int ddVar, bool val, const Cudd* mgr) const {
  if (ddPackage == CUDD) {
    if (util::isFound(ddVar, cuadd.SupportIndices())) {
      return Dd(cuadd.Compose(val ? mgr->addOne() : mgr->addZero(), ddVar));
    }
    return *this;
  }
  sylvan::MtbddMap m;
  m.put(ddVar, val ? Mtbdd::mtbddOne() : Mtbdd::mtbddZero());
  return Dd(mtbdd.Compose(m));
}

Dd Dd::getProduct(const Dd& dd) const {
  if (ddPackage == CUDD) {
    return logCounting ? Dd(cuadd + dd.cuadd) : Dd(cuadd * dd.cuadd);
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_times(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd * dd.mtbdd);
}

Dd Dd::getSum(const Dd& dd) const {
  if (ddPackage == CUDD) {
    return logCounting ? Dd(cuadd.LogSumExp(dd.cuadd)) : Dd(cuadd + dd.cuadd);
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_plus(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd + dd.mtbdd);
}

Dd Dd::getMax(const Dd& dd) const {
  if (ddPackage == CUDD) {
    return Dd(cuadd.Maximum(dd.cuadd));
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_max(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd.Max(dd.mtbdd));
}

Set<Int> Dd::getSupport() const {
  Set<Int> support;
  if (ddPackage == CUDD) {
    for (Int ddVar : cuadd.SupportIndices()) {
      support.insert(ddVar);
    }
  }
  else {
    Mtbdd cube = mtbdd.Support(); // conjunction of all vars appearing in mtbdd
    while (!cube.isOne()) {
      support.insert(cube.TopVar());
      cube = cube.Then();
    }
  }
  return support;
}

Dd Dd::getAbstraction(Int ddVar, const vector<Int>& ddVarToCnfVarMap, const Map<Int, Number>& literalWeights, const Assignment& assignment, bool additive, const Cudd* mgr) const {
  Int cnfVar = ddVarToCnfVarMap.at(ddVar);
  Dd positiveWeight = getConstDd(literalWeights.at(cnfVar), mgr);
  Dd negativeWeight = getConstDd(literalWeights.at(-cnfVar), mgr);

  auto it = assignment.find(cnfVar);
  if (it != assignment.end()) {
    Dd weight = it->second ? positiveWeight : negativeWeight;
    return getProduct(weight);
  }
  Dd term0 = getComposition(ddVar, false, mgr).getProduct(negativeWeight);
  Dd term1 = getComposition(ddVar, true, mgr).getProduct(positiveWeight);
  return additive ? term0.getSum(term1) : term0.getMax(term1);
}

void Dd::writeDotFile(const Cudd* mgr, string dotFileDir) const {
  string filePath = dotFileDir + "dd" + to_string(dotFileIndex++) + ".dot";
  FILE* file = fopen(filePath.c_str(), "wb"); // writes to binary file
  if (ddPackage == CUDD) { // davidkebo.com/cudd#cudd6
    DdNode** ddNodeArray = static_cast<DdNode**>(malloc(sizeof(DdNode*)));
    ddNodeArray[0] = cuadd.getNode();
    Cudd_DumpDot(mgr->getManager(), 1, ddNodeArray, NULL, NULL, file);
    free(ddNodeArray);
  }
  else {
    mtbdd_fprintdot_nc(file, mtbdd.GetMTBDD());
  }
  fclose(file);
  cout << "c overwrote file " << filePath << "\n";
}

/* class Executor =========================================================== */

Dd Executor::getClauseDd(const Map<Int, Int>& cnfVarToDdVarMap, const Clause& clause, const Cudd* mgr, const Assignment& assignment) {
  Dd clauseDd = Dd::getZeroDd(mgr);
  for (Int literal : clause) {
    Int cnfVar = Clause::getVar(literal);
    bool val = Clause::isPositiveLiteral(literal);
    auto it = assignment.find(cnfVar);
    if (it != assignment.end()) { // slices clause on literal
      if (it->second == val) { // returns satisfied clause
        return Dd::getOneDd(mgr);
      } // excludes unsatisfied literal from clause otherwise
    }
    else {
      Int ddVar = cnfVarToDdVarMap.at(cnfVar);
      Dd literalDd = Dd::getVarDd(ddVar, val, mgr);
      clauseDd = clauseDd.getMax(literalDd);
    }
  }
  return clauseDd;
}

Dd Executor::countSubtree(const JoinNode* joinNode, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr, const Assignment& assignment) {
  if (joinNode->isTerminal()) {
    Dd c = getClauseDd(cnfVarToDdVarMap, JoinNode::cnf.clauses.at(joinNode->nodeIndex), mgr, assignment);
    return c;
  }
  Dd dd = Dd::getOneDd(mgr);
  if (joinPriority == ARBITRARY_PAIR) { // arbitrarily multiplies child ADDs
    for (JoinNode* child : joinNode->children) {
      dd = dd.getProduct(countSubtree(child, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr, assignment));
    }
  }
  else { // Dd::operator< handles both biggest-first and smallest-first
    std::priority_queue<Dd> childDds;
    for (JoinNode* child : joinNode->children) {
      childDds.push(countSubtree(child, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr, assignment));
    }
    assert(!childDds.empty());
    while (childDds.size() > 1) {
      Dd dd1 = childDds.top();
      childDds.pop();
      Dd dd2 = childDds.top();
      childDds.pop();
      Dd dd3 = dd1.getProduct(dd2);
      childDds.push(dd3);
    }
    dd = childDds.top();
  }
  for (Int cnfVar : joinNode->projectionVars) {
    Int ddVar = cnfVarToDdVarMap.at(cnfVar);
    dd = dd.getAbstraction(ddVar, ddVarToCnfVarMap, JoinNode::cnf.literalWeights, assignment, JoinNode::cnf.additiveVars.contains(cnfVar), mgr);
  }
  return dd;
}

void Executor::countSlicedCnf(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr, const Assignment& assignment, Int threadIndex, Float& totalCount, mutex& threadMutex) {
  TimePoint start = util::getTimePoint();

  threadMutex.lock();
  cout << "c thread " << right << setw(5) << threadIndex << " | assignment { ";
  assignment.printAssignment();
  cout << " }\n";
  threadMutex.unlock();

  Float modelCount = countSubtree(static_cast<const JoinNode*>(joinRoot), cnfVarToDdVarMap, ddVarToCnfVarMap, mgr, assignment).extractConst().fraction;

  const std::lock_guard<mutex> g(threadMutex);
  cout << "c thread " << right << setw(5) << threadIndex << " | time " << right << setw(5) << static_cast<Int>(util::getSeconds(start)) << " | count " << left << setw(15);
  if (logCounting) {
    cout << exp(modelCount) << " | ln(count) " << modelCount << "\n";
    if (totalCount == -INF) {
      totalCount = modelCount;
    }
    else if (modelCount != -INF) {
      Float m = max(modelCount, totalCount);
      totalCount = log(exp(modelCount - m) + exp(totalCount - m)) + m;
    }
  }
  else {
    cout << modelCount << "\n";
    totalCount += modelCount;
  }
}

Number Executor::countCnf(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, Int sliceVarOrderHeuristic) {
  if (ddPackage == SYLVAN || threadCount < 2) {
    return countSubtree(
      static_cast<const JoinNode*>(joinRoot),
      cnfVarToDdVarMap,
      ddVarToCnfVarMap,
      ddPackage == CUDD ? Dd::newMgr(maxMegabytes) : nullptr
    ).extractConst();
  }

  vector<Assignment> assignments = joinRoot->getAdditiveAssignments(sliceVarOrderHeuristic, threadCount);

  util::printRow("sliceWidth", joinRoot->getWidth(assignments.front())); // any assignment would work

  Int threadMegabytes = maxMegabytes / assignments.size();
  util::printRow("threadMegabytes", threadMegabytes);

  Float totalCount = logCounting ? -INF : 0;
  mutex threadMutex;
  vector<thread> threads;

  for (Int i = 0; i < assignments.size() - 1; i++) {
    threads.push_back(thread(
      countSlicedCnf,
      cref(joinRoot),
      cref(cnfVarToDdVarMap),
      cref(ddVarToCnfVarMap),
      Dd::newMgr(threadMegabytes),
      cref(assignments.at(i)),
      i,
      ref(totalCount),
      ref(threadMutex)
    ));
  }
  countSlicedCnf(
    joinRoot,
    cnfVarToDdVarMap,
    ddVarToCnfVarMap,
    Dd::newMgr(threadMegabytes),
    assignments.back(),
    assignments.size() - 1,
    totalCount,
    threadMutex
  );
  for (thread& t : threads) {
    t.join();
  }

  return totalCount;
}

void Executor::printSolutionLine(const Number& modelCount) {
  cout << THIN_LINE;
  string solutionWord = weightFormat == PROJECTED ? "pmc" : (weightFormat == WEIGHTED ? "wmc" : "mc");

  if (logCounting) {
    util::printRow(solutionWord, exp(modelCount.fraction), "s");
    util::printRow("ln(" + solutionWord + ")", modelCount);
  }
  else {
    util::printRow(solutionWord, modelCount, "s");
  }

  if (multiplePrecision) {
    Float f = modelCount.getFloat();
    util::printRow("stold(" + solutionWord + ")", f);
    if (weightFormat == UNWEIGHTED) {
      util::printRow("stoll(" + solutionWord + ")", static_cast<Int>(f));
    }
  }

  cout << THIN_LINE;
}

Executor::Executor(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, Int sliceVarOrderHeuristic) {
  cout << "\n";
  cout << "c computing output...\n";
  util::setSignalHandler();
  Map<Int, Int> cnfVarToDdVarMap; // e.g. {42: 0, 13: 1}

  TimePoint start = util::getTimePoint();
  vector<Int> ddVarToCnfVarMap = joinRoot->getVarOrder(ddVarOrderHeuristic); // e.g. [42, 13], i.e. ddVarOrder
  if (verbosityLevel > 0) {
    util::printRow("diagramVarSeconds", util::getSeconds(start));
  }

  for (Int ddVar = 0; ddVar < ddVarToCnfVarMap.size(); ddVar++) {
    Int cnfVar = ddVarToCnfVarMap.at(ddVar);
    cnfVarToDdVarMap[cnfVar] = ddVar;
  }
  Number n = countCnf(joinRoot, cnfVarToDdVarMap, ddVarToCnfVarMap, sliceVarOrderHeuristic);
  for (Int var = 1; var <= JoinNode::cnf.declaredVarCount; var++) {
    if (!JoinNode::cnf.apparentVars.contains(var)) {
      if (logCounting) {
        n += Number((JoinNode::cnf.literalWeights.at(var) + JoinNode::cnf.literalWeights.at(-var)).getLn());
      }
      else {
        n *= JoinNode::cnf.literalWeights.at(var) + JoinNode::cnf.literalWeights.at(-var);
      }
    }
  }
  printSolutionLine(n);
}

/* class OptionDict ========================================================= */

string OptionDict::helpPlanningStrategy() {
  string s = "planning strategy: ";
  for (auto it = PLANNING_STRATEGIES.begin(); it != PLANNING_STRATEGIES.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != PLANNING_STRATEGIES.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

string OptionDict::helpDdPackage() {
  string s = "diagram package: ";
  for (auto it = DD_PACKAGES.begin(); it != DD_PACKAGES.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != DD_PACKAGES.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

string OptionDict::helpJoinPriority() {
  string s = "join priority: ";
  for (auto it = JOIN_PRIORITIES.begin(); it != JOIN_PRIORITIES.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != JOIN_PRIORITIES.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

void OptionDict::solveOptions() const {
  if (ddPackage == SYLVAN) {
    lace_init(threadCount, 0);
    lace_startup(0, NULL, NULL);
    sylvan::sylvan_set_limits(maxMegabytes * 1024 * 1024, tableRatio, initRatio);
    sylvan::sylvan_init_package();
    sylvan::sylvan_init_mtbdd();
    if (multiplePrecision) {
      sylvan::gmp_init();
    }
  }
  if (verbosityLevel > 0) {
    cout << "c processing command-line options...\n";
    util::printRow("cnfFile", cnfFilePath);
    util::printRow("weightFormat", WEIGHT_FORMATS.at(weightFormat));

    util::printRow("planningStrategy", PLANNING_STRATEGIES.at(planningStrategy));
    if (planningStrategy == TIMED_JOINTREES) {
      util::printRow("jointreeWaitSeconds", jtWaitSeconds);
    }

    util::printRow("threadCount", threadCount);

    util::printRow("diagramPackage", DD_PACKAGES.at(ddPackage));

    util::printRow("randomSeed", randomSeed);

    util::printRow("diagramVarOrder", (ddVarOrderHeuristic < 0 ? "INVERSE_" : "") + CNF_VAR_ORDER_HEURISTICS.at(abs(ddVarOrderHeuristic)));

    if (ddPackage == CUDD && threadCount > 1) {
      util::printRow("sliceVarOrder", (sliceVarOrderHeuristic < 0 ? "INVERSE_" : "") + util::getVarOrderHeuristics().at(abs(sliceVarOrderHeuristic)));
    }

    util::printRow("maxMegabytes", maxMegabytes);

    if (ddPackage == SYLVAN) {
      util::printRow("initRatio", initRatio);
      util::printRow("tableRatio", tableRatio);
      util::printRow("multiplePrecision", multiplePrecision);
    }
    else {
      util::printRow("logCounting", logCounting);
    }

    util::printRow("joinPriority", JOIN_PRIORITIES.at(joinPriority));
    cout << "\n";
  }
  JoinNode::cnf = Cnf(cnfFilePath);
  JoinTreeProcessor* joinTreeProcessor;
  if (planningStrategy == FIRST_JOINTREE) {
    joinTreeProcessor = new JoinTreeParser();
  }
  else {
    joinTreeProcessor = new JoinTreeReader(jtWaitSeconds);
  }
  Executor executor(joinTreeProcessor->getJoinTreeRoot(), ddVarOrderHeuristic, sliceVarOrderHeuristic);
  if (ddPackage == SYLVAN) {
    sylvan::sylvan_stats_report(stdout);
    sylvan::sylvan_quit();
    lace_exit();
  }
}

OptionDict::OptionDict(int argc, char** argv) {
  options = new cxxopts::Options("dmc", "Diagram Model Counter (reads jointree from STDIN)");
  options->add_options()
    (CNF_FILE_OPTION, "CNF file path; string (REQUIRED)", value<string>())
    (WEIGHT_FORMAT_OPTION, Cnf::helpWeightFormat(), value<string>()->default_value(PROJECTED))
    (PLANNING_STRATEGY_OPTION, helpPlanningStrategy(), value<string>()->default_value(FIRST_JOINTREE))
    (JT_WAIT_OPTION, "jointree wait seconds before killing planner [with " + PLANNING_STRATEGY_OPTION + "_arg == " + TIMED_JOINTREES + "]; float", value<Float>()->default_value("0.5"))
    (THREAD_COUNT_OPTION, "thread count, or '0' for hardware_concurrency value; int", value<Int>()->default_value("1"))
    (DD_PACKAGE_OPTION, helpDdPackage(), value<string>()->default_value(CUDD))
    (RANDOM_SEED_OPTION, "random seed; int", value<Int>()->default_value("2020"))
    (DD_VAR_OPTION, util::helpVarOrderHeuristic("diagram"), value<Int>()->default_value(to_string(MCS)))
    (SLICE_VAR_OPTION, util::helpVarOrderHeuristic("slice"), value<Int>()->default_value(to_string(BIGGEST_NODE)))
    (MAX_MEM_OPTION, "max megabytes for unique table and cache table combined; int", value<Int>()->default_value("1024"))
    (INIT_RATIO_OPTION, "init ratio for tables [with " + DD_PACKAGE_OPTION + "_arg == " + SYLVAN + "]: log2(max_size/init_size); int", value<Int>()->default_value("0"))
    (TABLE_RATIO_OPTION, "table ratio [with " + DD_PACKAGE_OPTION + "_arg == " + SYLVAN + "]: log2(unique_table_size/cache_table_size); int", value<Int>()->default_value("0"))
    (MULTIPLE_PRECISION_OPTION, "multiple precision [with " + DD_PACKAGE_OPTION + "_arg == " + SYLVAN + "]: 0, 1; int", value<Int>()->default_value("0"))
    (LOG_COUNTING_OPTION, "log counting [with " + DD_PACKAGE_OPTION + "_arg == " + CUDD + "]: 0, 1; int", value<Int>()->default_value("0"))
    (JOIN_PRIORITY_OPTION, helpJoinPriority(), value<string>()->default_value(SMALLEST_PAIR))
    (VERBOSITY_LEVEL_OPTION, util::helpVerbosity(), value<Int>()->default_value("1"))
  ;
  cxxopts::ParseResult result = options->parse(argc, argv);
  if (result.count(CNF_FILE_OPTION)) {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();

    weightFormat = result[WEIGHT_FORMAT_OPTION].as<string>(); // global var
    assert(WEIGHT_FORMATS.contains(weightFormat));

    planningStrategy = result[PLANNING_STRATEGY_OPTION].as<string>(); // global var
    assert(PLANNING_STRATEGIES.contains(planningStrategy));

    jtWaitSeconds = result[JT_WAIT_OPTION].as<Float>();

    threadCount = result[THREAD_COUNT_OPTION].as<Int>(); // global var
    if (threadCount <= 0) {
      threadCount = thread::hardware_concurrency();
    }
    assert(threadCount > 0);

    ddPackage = result[DD_PACKAGE_OPTION].as<string>(); // global var
    assert(DD_PACKAGES.contains(ddPackage));

    randomSeed = result[RANDOM_SEED_OPTION].as<Int>(); // global var

    ddVarOrderHeuristic = result[DD_VAR_OPTION].as<Int>();
    assert(CNF_VAR_ORDER_HEURISTICS.contains(abs(ddVarOrderHeuristic)));

    sliceVarOrderHeuristic = result[SLICE_VAR_OPTION].as<Int>();
    assert(util::getVarOrderHeuristics().contains(abs(sliceVarOrderHeuristic)));

    maxMegabytes = result[MAX_MEM_OPTION].as<Int>(); // global var

    tableRatio = result[TABLE_RATIO_OPTION].as<Int>();

    initRatio = result[INIT_RATIO_OPTION].as<Int>();

    multiplePrecision = result[MULTIPLE_PRECISION_OPTION].as<Int>(); // global var
    assert(!multiplePrecision || ddPackage == SYLVAN);

    logCounting = result[LOG_COUNTING_OPTION].as<Int>(); // global var
    assert(!logCounting || ddPackage == CUDD);

    joinPriority = result[JOIN_PRIORITY_OPTION].as<string>(); //global var
    assert(JOIN_PRIORITIES.contains(joinPriority));

    verbosityLevel = result[VERBOSITY_LEVEL_OPTION].as<Int>(); // global var

    startTime = util::getTimePoint(); // global var
    solveOptions();
    util::printRow("seconds", util::getSeconds(startTime));
  }
  else {
    cout << options->help();
  }
}

/* global functions ========================================================= */

int main(int argc, char** argv) {
  cout << std::unitbuf; // enables automatic flushing
  OptionDict(argc, argv);
  // util::checkTypeSizes();
}
