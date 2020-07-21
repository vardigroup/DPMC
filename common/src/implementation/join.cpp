/* inclusions *****************************************************************/

#include "../interface/join.hpp"

/* constants ******************************************************************/

const string &JOIN_TREE_END_WORD = "=";
const string &JT_WORD = "jt";
const string &VAR_ELIM_WORD = "e";

/* classes ********************************************************************/

/* class JoinNode *********************************************************/

Int JoinNode::backupNodeCount;
Int JoinNode::backupTerminalCount;
Set<Int> JoinNode::backupNonterminalIndexes;

Int JoinNode::nodeCount;
Int JoinNode::terminalCount;
Set<Int> JoinNode::nonterminalIndexes;

void JoinNode::resetStaticFields() {
  backupNodeCount = nodeCount;
  backupTerminalCount = terminalCount;
  backupNonterminalIndexes = nonterminalIndexes;

  nodeCount = 0;
  terminalCount = 0;
  nonterminalIndexes.clear();
}

void JoinNode::restoreStaticFields() {
  nodeCount = backupNodeCount;
  terminalCount = backupTerminalCount;
  nonterminalIndexes = backupNonterminalIndexes;
}

Int JoinNode::getNodeCount() {
  return nodeCount;
}

Int JoinNode::getTerminalCount() {
  return terminalCount;
}

bool JoinNode::isTerminal() const {
  return nodeIndex < terminalCount;
}

Int JoinNode::getNodeIndex() const {
  return nodeIndex;
}

const vector<JoinNode *> &JoinNode::getChildren() const {
  return children;
}

const Set<Int> &JoinNode::getProjectableCnfVars() const {
  return projectableCnfVars;
}

/* class JoinTerminal *****************************************************/

const Set<Int> &JoinTerminal::getApparentCnfVars(const vector<vector<Int>> &clauses) {
  if (!evaluatedApparentCnfVars) {
    evaluatedApparentCnfVars = true;
    apparentCnfVars = util::getClauseCnfVars(clauses.at(nodeIndex));
  }
  return apparentCnfVars;
}

Int JoinTerminal::getMaxVarCount(const vector<vector<Int>> &clauses) {
  return getApparentCnfVars(clauses).size();
}

void JoinTerminal::printSubtree(const string &prefix) const {}

JoinTerminal::JoinTerminal() {
  nodeIndex = terminalCount;
  terminalCount++;
  nodeCount++;
}

/* class JoinNonterminal **************************************************/

void JoinNonterminal::printNode(const string &prefix = "") const {
  cout << prefix << nodeIndex + 1 << " ";

  for (JoinNode *child : children) {
    cout << child->getNodeIndex() + 1 << " ";
  }

  cout << VAR_ELIM_WORD << " ";
  for (Int cnfVar : projectableCnfVars) {
    cout << cnfVar << " ";
  }

  cout << "\n";
}

const Set<Int> &JoinNonterminal::getApparentCnfVars(const vector<vector<Int>> &clauses) {
  if (!evaluatedApparentCnfVars) {
    evaluatedApparentCnfVars = true;
    for (JoinNode *child : children) {
      for (Int cnfVar : child->getApparentCnfVars(clauses)) {
        if (!util::isFound(cnfVar, child->getProjectableCnfVars())) {
          apparentCnfVars.insert(cnfVar);
        }
      }
    }
  }
  return apparentCnfVars;
}

Int JoinNonterminal::getMaxVarCount(const vector<vector<Int>> &clauses) {
  Int maxVarCount = getApparentCnfVars(clauses).size();
  for (JoinNode *child : children) {
    Int subtreeMaxVarCount = child->getMaxVarCount(clauses);
    maxVarCount = std::max(maxVarCount, subtreeMaxVarCount);
  }
  return maxVarCount;
}

void JoinNonterminal::printSubtree(const string &prefix) const {
  for (JoinNode *child : children) {
    child->printSubtree(prefix);
  }
  printNode(prefix);
}

void JoinNonterminal::addProjectableCnfVars(const Set<Int> &cnfVars) {
  util::unionize(projectableCnfVars, cnfVars);
}

JoinNonterminal::JoinNonterminal(const vector<JoinNode *> &children, const Set<Int> &projectableCnfVars, Int requestedNodeIndex) {
  this->children = children;
  this->projectableCnfVars = projectableCnfVars;

  if (requestedNodeIndex == DUMMY_MIN_INT) {
    requestedNodeIndex = nodeCount;
  }
  else if (requestedNodeIndex < terminalCount) {
    showError("requestedNodeIndex = " + to_string(requestedNodeIndex) + " < " + to_string(terminalCount) + " = terminalCount");
  }
  else if (util::isFound(requestedNodeIndex, nonterminalIndexes)) {
    showError("requestedNodeIndex = " + to_string(requestedNodeIndex) + " already taken");
  }
  nodeIndex = requestedNodeIndex;
  nonterminalIndexes.insert(requestedNodeIndex);
  nodeCount++;
}

/* class JoinTree *************************************************************/

JoinNode *JoinTree::getJoinNode(Int nodeIndex) const {
  auto it = joinTerminals.find(nodeIndex);
  if (it != joinTerminals.end()) {
    return it->second;
  }
  else {
    return joinNonterminals.at(nodeIndex);
  }
}

JoinNonterminal *JoinTree::getJoinRoot() const {
  return joinNonterminals.at(declaredNodeCount - 1);
}

void JoinTree::printTree() const {
  printComment(PROBLEM_WORD + " " + JT_WORD + " " + to_string(declaredVarCount) + " " + to_string(declaredClauseCount) + " " + to_string(declaredNodeCount));
  getJoinRoot()->printSubtree("c\t");
}

JoinTree::JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount) {
  this->declaredVarCount = declaredVarCount;
  this->declaredClauseCount = declaredClauseCount;
  this->declaredNodeCount = declaredNodeCount;
}

/* class JoinTreeReader ***********************************************************/

Float JoinTreeReader::timeoutSeconds;
TimePoint JoinTreeReader::startPoint;
Int JoinTreeReader::plannerPid = DUMMY_MIN_INT;

void JoinTreeReader::killPlanner() {
  if (plannerPid == DUMMY_MIN_INT) {
    showWarning("plannerPid == DUMMY_MIN_INT; cannot kill");
  }
  else if (kill(plannerPid, SIGKILL) == 0) { // SIGTERM is not fast enough
    printComment("Successfully killed planner process with PID " + to_string(plannerPid));
  }
  else {
    showWarning("failed to kill planner processs with PID " + to_string(plannerPid));
  }
}

void JoinTreeReader::handleAlarm(int signal) {
  if (signal != SIGALRM) {
    showError("signal == " + to_string(signal) + " != " + to_string(SIGALRM) + " == SIGALRM");
  }

  printThinLine();
  printComment("Received SIGALRM after " + to_string(getSeconds(startTime)) + " seconds since the main programm started");
  killPlanner();
}

void JoinTreeReader::setAlarm(Float seconds) {
  if (seconds < 0) {
    util::showError("seconds == " + to_string(seconds) + " < 0");
  }

  Int secs = seconds;
  Int usecs = (seconds - secs) * 1000000;

  struct itimerval new_value;
  new_value.it_value.tv_sec = secs;
  new_value.it_value.tv_usec = usecs;
  new_value.it_interval.tv_sec = 0;
  new_value.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &new_value, nullptr);
}

bool JoinTreeReader::isDisarmed() {
  struct itimerval curr_value;
  getitimer(ITIMER_REAL, &curr_value);
  return curr_value.it_value.tv_sec == 0 && curr_value.it_value.tv_usec == 0 && curr_value.it_interval.tv_sec == 0 && curr_value.it_interval.tv_usec == 0;
}

void JoinTreeReader::disarmAlarm() {
  setAlarm(0);
  util::printComment("Disarmed alarm");
}

void JoinTreeReader::reviseAlarm(Float newTimeoutSeconds) {
  Float remainingSeconds = timeoutSeconds - util::getSeconds(startPoint);

  if (remainingSeconds <= 0 && !isDisarmed()) {
    util::showWarning("alarm should have been triggered " + to_string(-remainingSeconds) + " seconds ago; calling SIGALRM handler now");
    handleAlarm(SIGALRM);
    return;
  }

  util::printComment("Remaining duration: " + to_string(remainingSeconds) + " seconds");

  cout << "c Requested new timeout: " << newTimeoutSeconds << " seconds";
  if (newTimeoutSeconds < remainingSeconds) {
    cout << " -- request granted\n";

    timeoutSeconds = newTimeoutSeconds;
    startPoint = util::getTimePoint();

    setAlarm(timeoutSeconds);
  }
  else {
    cout << " -- request declined\n";
  }
}

void JoinTreeReader::initializeAlarm(Float seconds) {
  if (seconds <= 0) {
    util::showError("seconds == " + to_string(seconds) + " <= 0");
  }

  timeoutSeconds = seconds;
  startPoint = util::getTimePoint();

  signal(SIGALRM, handleAlarm);
  setAlarm(seconds);

  util::printComment("Constructed timer with " + to_string(seconds) + "-second timeout");
}

Float JoinTreeReader::getNewTimeoutSeconds(Int maxVarCount) const {
  Float ddOperations = pow(2, maxVarCount); // big-O approximation
  Float newTimeoutSeconds = ddOperations * performanceFactor;
  return newTimeoutSeconds;
}

void JoinTreeReader::finishReadingJoinTree(bool alarming) {
  if (joinTree == nullptr) {
    showError("no join tree ending on or before line " + to_string(lineIndex));
  }

  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;
  if (nonterminalCount < expectedNonterminalCount) {
    showWarning("missing branch nodes (" + to_string(nonterminalCount) + " found, " + to_string(expectedNonterminalCount) + " expected) before current join tree ends on line " + to_string(lineIndex));

    if (joinTreeEndLineIndex == DUMMY_MIN_INT) {
      showError("no backup join tree");
    }
    else {
      showWarning("restoring backup join tree ending on line " + to_string(joinTreeEndLineIndex));
      if (verbosityLevel >= 1) {
        printThinLine();
        printComment("Restored backup join tree:");
        backupJoinTree->printTree();
        printThinLine();
      }

      joinTree = backupJoinTree;
      JoinNode::restoreStaticFields();
    }
  }
  else {
    Int maxVarCount = joinTree->getJoinRoot()->getMaxVarCount(clauses);

    printThinLine();
    printComment("After " + to_string(getSeconds(startTime)) + " seconds, finished reading last legal join tree ending on or before line " + to_string(lineIndex) + " with ADD width " + to_string(maxVarCount));

    if (alarming) {
      Float newTimeoutSeconds = getNewTimeoutSeconds(maxVarCount);
      reviseAlarm(newTimeoutSeconds);
    }

    if (verbosityLevel >= 2) {
      printThinLine();
      joinTree->printTree();
      printThinLine();
    }
  }
  joinTreeEndLineIndex = lineIndex;
  problemLineIndex = DUMMY_MIN_INT;
}

void JoinTreeReader::readInputStream(std::istream *inputStream) {
  string line;
  while (
    std::getline(*inputStream, line)
    && (inputStream != &std::cin || !isDisarmed())
  ) {
    lineIndex++;
    std::istringstream inputStringStream(line);

    if (verbosityLevel >= 3) printComment("Line " + to_string(lineIndex) + "\t" + line);

    vector<string> words;
    std::copy(std::istream_iterator<string>(inputStringStream), std::istream_iterator<string>(), std::back_inserter(words));

    Int wordCount = words.size();
    if (wordCount < 1) continue;
    const string &startWord = words.at(0);

    bool readingElimVars = false;
    if (startWord == JOIN_TREE_END_WORD) {
      finishReadingJoinTree(inputStream == &std::cin);
    }
    else if (startWord == PROBLEM_WORD) {
      if (problemLineIndex != DUMMY_MIN_INT) {
        showError("multiple problem lines: " + to_string(problemLineIndex) + " and " + to_string(lineIndex));
      }
      problemLineIndex = lineIndex;

      if (wordCount != 5) {
        showError("problem line " + to_string(lineIndex) + " has " + to_string(wordCount) + " words (should be 5)");
      }
      const string &jtWord = words.at(1);
      if (jtWord != JT_WORD) {
        showError("expected '" + JT_WORD + "', found '" + jtWord + "' -- line " + to_string(lineIndex));
      }

      Int declaredVarCount = std::stoll(words.at(2));
      Int declaredClauseCount = std::stoll(words.at(3));
      Int declaredNodeCount = std::stoll(words.at(4));

      backupJoinTree = joinTree;
      joinTree = new JoinTree(declaredVarCount, declaredClauseCount, declaredNodeCount);

      JoinNode::resetStaticFields();
      for (Int terminalIndex = 0; terminalIndex < declaredClauseCount; terminalIndex++) {
        joinTree->joinTerminals[terminalIndex] = new JoinTerminal();
      }
    }
    else if (startWord == "c") { // comment
      if (wordCount == 3) {
        const string &key = words.at(1);
        const string &value = words.at(2);
        if (key == "pid") {
          JoinTreeReader::plannerPid = std::stoll(value);
        }
        else if (key == "seconds") {
          if (joinTree != nullptr) {
            joinTree->plannerSeconds = std::stold(value);
          }
        }
      }
    }
    else { // branch node line
      if (problemLineIndex == DUMMY_MIN_INT) {
        string message = "no problem line before branch node line " + to_string(lineIndex);
        if (joinTreeEndLineIndex != DUMMY_MIN_INT) {
          message += " (last legal join tree ends on line " + to_string(joinTreeEndLineIndex) + ")";
        }
        showError(message);
      }

      Int parentIndex = std::stoll(startWord) - 1; // 0-indexing
      if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
        showError("wrong branch node index -- line " + to_string(lineIndex));
      }

      vector<JoinNode *> children;
      Set<Int> projectableCnfVars;
      for (Int i = 1; i < wordCount; i++) {
        const string &word = words.at(i);
        if (word == VAR_ELIM_WORD) {
          readingElimVars = true;
        }
        else {
          Int num = std::stoll(word);
          if (readingElimVars) {
            Int declaredVarCount = joinTree->declaredVarCount;
            if (num <= 0 || num > declaredVarCount) {
              showError("var '" + to_string(num) + "' is inconsistent with declared var count '" + to_string(declaredVarCount) + "' -- line " + to_string(lineIndex));
            }
            projectableCnfVars.insert(num);
          }
          else {
            int childIndex = num - 1; // 0-indexing
            if (childIndex < 0 || childIndex >= parentIndex) {
              showError("child '" + word + "' is wrong -- line " + to_string(lineIndex));
            }
            children.push_back(joinTree->getJoinNode(childIndex));
          }
        }
      }
      joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectableCnfVars, parentIndex);
    }
  }

  if (inputStream == &std::cin && !isDisarmed()) {
    showWarning("alarm should have been disarmed; disarming alarm now");
    disarmAlarm();
  }

  finishReadingJoinTree(false);
}

JoinNonterminal *JoinTreeReader::getJoinTreeRoot() const {
  return joinTree->getJoinRoot();
}

JoinTreeReader::JoinTreeReader(
  const string &filePath,
  Float jtWaitSeconds,
  Float performanceFactor,
  const vector<vector<Int>> &clauses
) {
  this->jtWaitSeconds = jtWaitSeconds;
  this->performanceFactor = performanceFactor;
  this->clauses = clauses;

  printComment("Reading join tree...", 1);

  std::ifstream inputFileStream(filePath); // variable will be destroyed if it goes out of scope
  std::istream *inputStream;
  if (filePath == STDIN_CONVENTION) {
    inputStream = &std::cin;
    initializeAlarm(jtWaitSeconds);

    printThickLine();
    printComment("Getting join tree from stdin with " + to_string(jtWaitSeconds) + "-second timeout... (end input with 'Enter' then 'Ctrl d')");
  }
  else {
    if (!inputFileStream.is_open()) {
      showError("unable to open file '" + filePath + "'");
    }
    inputStream = &inputFileStream;
  }

  readInputStream(inputStream);

  if (filePath == STDIN_CONVENTION) {
    if (plannerPid != DUMMY_MIN_INT && kill(plannerPid, 0) == 0) {
      showWarning("planner should have been killed; killing it now");
      killPlanner();
    }

    printComment("Getting join tree from stdin: done");
    printThickLine();
  }

  if (verbosityLevel >= 1) {
    util::printRow("declaredVarCount", joinTree->declaredVarCount);
    util::printRow("declaredClauseCount", joinTree->declaredClauseCount);
    util::printRow("declaredNodeCount", joinTree->declaredNodeCount);
    util::printRow("plannerSeconds", joinTree->plannerSeconds);
  }
}
