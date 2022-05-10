/* inclusions *****************************************************************/

#include "../interface/join.hpp"

/* constants ******************************************************************/

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

const Set<Int> &JoinNode::getProjectableVars() const {
  return projectableVars;
}

Int JoinNode::chooseClusterIndex(Int clusterIndex, vector<Set<Int>> projectableVarSets, ClusteringHeuristic clusteringHeuristic, const vector<vector<Int>> &clauses) {
  if (clusterIndex < 0 || clusterIndex >= projectableVarSets.size()) {
    showError("clusterIndex == " + to_string(clusterIndex) + " whereas projectableVarSets.size() == " + to_string(projectableVarSets.size()));
  }

  Set<Int> projectableVars = util::getUnion(projectableVarSets); // Z = Z_1 \cup ... \cup Z_m
  Set<Int> postProjectionVars = getPostProjectionVars(clauses); // of this node
  if (util::isDisjoint(projectableVars, postProjectionVars)) {
    return projectableVarSets.size(); // special cluster
  }

  switch (clusteringHeuristic) {
    case ClusteringHeuristic::BUCKET_LIST: {} // falls through
    case ClusteringHeuristic::BOUQUET_LIST: {
      return clusterIndex + 1;
    }
    case ClusteringHeuristic::BUCKET_TREE: {} // falls through
    case ClusteringHeuristic::BOUQUET_TREE: {
      for (Int target = clusterIndex + 1; target < projectableVarSets.size(); target++) {
        if (!util::isDisjoint(postProjectionVars, projectableVarSets.at(target))) {
          return target;
        }
      }
      return projectableVarSets.size();
    }
    default: {
      showError("unsupported clusteringHeuristic: " + util::getClusteringHeuristicName(clusteringHeuristic) + " | JoinNode::chooseClusterIndex");
      return DUMMY_MIN_INT; // unreachable line
    }
  }
}

Int JoinNode::getNodeRank(const vector<Int> &restrictedVarOrdering, ClusteringHeuristic clusteringHeuristic, const vector<vector<Int>> &clauses) {
  const Set<Int> &postProjectionVars = getPostProjectionVars(clauses);
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::BUCKET_LIST: {} // falls through
    case ClusteringHeuristic::BUCKET_TREE: { // min var rank
      Int rank = DUMMY_MAX_INT;
      for (Int varRank = 0; varRank < restrictedVarOrdering.size(); varRank++) {
        if (util::isFound(restrictedVarOrdering.at(varRank), postProjectionVars)) {
          rank = std::min(rank, varRank);
        }
      }
      return (rank == DUMMY_MAX_INT) ? restrictedVarOrdering.size() : rank;
    }
    case ClusteringHeuristic::BOUQUET_LIST: {} // falls through
    case ClusteringHeuristic::BOUQUET_TREE: { // max var rank
      Int rank = DUMMY_MIN_INT;
      for (Int varRank = 0; varRank < restrictedVarOrdering.size(); varRank++) {
        if (util::isFound(restrictedVarOrdering.at(varRank), postProjectionVars)) {
          rank = std::max(rank, varRank);
        }
      }
      return (rank == DUMMY_MIN_INT) ? restrictedVarOrdering.size() : rank;
    }
    default: {
      showError("unsupported clusteringHeuristic: " + util::getClusteringHeuristicName(clusteringHeuristic) + " | JoinNode::getNodeRank");
      return -1; // unreachable line
    }
  }
}

Set<Int> JoinNode::getPostProjectionVars(const vector<vector<Int>> &clauses) {
  Set<Int> vars;
  util::differ(vars, getPreProjectionVars(clauses), projectableVars);
  return vars;
}

/* class JoinTerminal *****************************************************/

const Set<Int> &JoinTerminal::getPreProjectionVars(const vector<vector<Int>> &clauses) {
  if (!evaluatedPreProjectionVars) {
    preProjectionVars = util::getClauseCnfVars(clauses.at(nodeIndex));
    evaluatedPreProjectionVars = true;
  }
  return preProjectionVars;
}

Int JoinTerminal::getMaxVarCount(const vector<vector<Int>> &clauses) {
  return getPreProjectionVars(clauses).size();
}

void JoinTerminal::printSubtree(const string &prefix) const {} // prints nothing for leaf

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

  cout << VAR_ELIM_WORD;
  for (Int var : projectableVars) {
    cout << " " << var;
  }

  cout << "\n";
}

const Set<Int> &JoinNonterminal::getPreProjectionVars(const vector<vector<Int>> &clauses) {
  if (!evaluatedPreProjectionVars) {
    for (JoinNode *child : children) {
      util::unionize(preProjectionVars, child->getPostProjectionVars(clauses));
    }
    evaluatedPreProjectionVars = true;
  }
  return preProjectionVars;
}

Int JoinNonterminal::getMaxVarCount(const vector<vector<Int>> &clauses) {
  Int maxVarCount = getPreProjectionVars(clauses).size();
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

void JoinNonterminal::addProjectableVars(const Set<Int> &vars) {
  util::unionize(projectableVars, vars);
}

JoinNonterminal::JoinNonterminal(const vector<JoinNode *> &children, const Set<Int> &projectableVars, Int requestedNodeIndex) {
  this->children = children;
  this->projectableVars = projectableVars;

  if (requestedNodeIndex == DUMMY_MIN_INT) {
    requestedNodeIndex = nodeCount;
  }
  else if (requestedNodeIndex < terminalCount) {
    showError("requestedNodeIndex == " + to_string(requestedNodeIndex) + " < " + to_string(terminalCount) + " == terminalCount");
  }
  else if (util::isFound(requestedNodeIndex, nonterminalIndexes)) {
    showError("requestedNodeIndex " + to_string(requestedNodeIndex) + " already taken");
  }
  nodeIndex = requestedNodeIndex;
  nonterminalIndexes.insert(requestedNodeIndex);
  nodeCount++;
}

/* class JoinComponent ********************************************************/

Set<Int> JoinComponent::getNodeVars(vector<JoinNode *> nodes) const {
  Set<Int> vars;
  for (JoinNode *node : nodes) {
    util::unionize(vars, node->getPostProjectionVars(cnf.getClauses()));
  }
  return vars;
}

void JoinComponent::computeFields() {
  util::differ(projectableVars, getNodeVars(subtrees), keptVars);

  nodeClusters = vector<vector<JoinNode *>>(projectableVars.size() + 1, vector<JoinNode *>());
  const vector<Int> &restrictedVarOrdering = cnf.getRestrictedVarOrdering(varOrderingHeuristic, inverseVarOrdering, projectableVars); // omega
  for (JoinNode *subtree : subtrees) {
    Int nodeRank = subtree->getNodeRank(restrictedVarOrdering, clusteringHeuristic, cnf.getClauses());
    nodeClusters.at(nodeRank).push_back(subtree);
  }

  projectableVarSets = vector<Set<Int>>(projectableVars.size(), Set<Int>());
  Set<Int> projectedVars; // accumulates Z_m..Z_i
  for (Int clusterIndex = projectableVars.size() - 1; clusterIndex >= 0; clusterIndex--) {
    Set<Int> projectableVarSet = util::getIntersection(projectableVars, getNodeVars(nodeClusters.at(clusterIndex)));
    util::differ(projectableVarSets.at(clusterIndex), projectableVarSet, projectedVars);
    util::unionize(projectedVars, projectableVarSet);
  }
}

JoinNonterminal *JoinComponent::getComponentRoot() {
  for (Int clusterIndex = 0; clusterIndex < projectableVars.size(); clusterIndex++) {
    const vector<JoinNode *> &children = nodeClusters.at(clusterIndex);
    if (!children.empty()) {
      JoinNonterminal *node = new JoinNonterminal(children, projectableVarSets.at(clusterIndex));
      Int target = node->chooseClusterIndex(clusterIndex, projectableVarSets, clusteringHeuristic, cnf.getClauses());
      nodeClusters.at(target).push_back(node);
    }
  }

  JoinNonterminal *componentRoot = new JoinNonterminal(nodeClusters.back());
  return componentRoot;
}

JoinComponent::JoinComponent(const Cnf &cnf, VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering, ClusteringHeuristic clusteringHeuristic, const vector<JoinNode *> &subtrees, const Set<Int> &keptVars) {
  this->cnf = cnf;
  this->varOrderingHeuristic = varOrderingHeuristic;
  this->inverseVarOrdering = inverseVarOrdering;
  this->clusteringHeuristic = clusteringHeuristic;
  this->subtrees = subtrees;
  this->keptVars = keptVars;

  computeFields();
}

/* class JoinRootBuilder ******************************************************/

void JoinRootBuilder::printDisjunctiveVarSets() const {
  cout << "c disjunctiveVars:";
  for (Int var : disjunctiveVars) {
    cout << " " << var;
  }
  cout << "\n";

  for (Int clauseIndex = 0; clauseIndex < disjunctiveVarSets.size(); clauseIndex++) {
    cout << "c disjunctiveVarSet for clause " << clauseIndex << ":";
    for (Int v : disjunctiveVarSets.at(clauseIndex)) {
      cout << " " << v;
    }
    cout << "\n";
  }
}

void JoinRootBuilder::printClauseGroups() const {
  for (Int i = 0; i < clauseGroups.size(); i++) {
    cout << "c clause group " << i << " contains these clause indexes:";
    for (Int clauseIndex : clauseGroups.at(i)) {
      cout << " " << clauseIndex;
    }
    cout << "\n";
  }
}

void JoinRootBuilder::setDisjunctiveVarSets() {  disjunctiveVars = cnf.getDisjunctiveVars();
  for (const vector<Int> &clause : cnf.getClauses()) {
    disjunctiveVarSets.push_back(util::getIntersection(util::getClauseCnfVars(clause), disjunctiveVars));
  }
}

void JoinRootBuilder::setClauseGroups() {
  Int boostSize = cnf.getDeclaredVarCount() + cnf.getClauses().size() + 10; // extra space to avoid memory errors
  vector<Int> rank(boostSize);
  vector<Int> parent(boostSize);
  boost::disjoint_sets<Int *, Int *> varBlocks(&rank.at(0), &parent.at(0));

  for (Int var : disjunctiveVars) {
    varBlocks.make_set(var);
  }

  for (Int i = 0; i < disjunctiveVarSets.size(); i++) {
    Int element = DUMMY_MIN_INT;
    for (Int var : disjunctiveVarSets.at(i)) {
      if (element != DUMMY_MIN_INT) {
        varBlocks.union_set(var, element);
      }
      element = var;
    }
  }

  Map<Int, vector<Int>> clauseMap; // representative var |-> clause indexes

  for (Int clauseIndex = 0; clauseIndex < disjunctiveVarSets.size(); clauseIndex++) {
    const Set<Int> &disjunctiveVarSet = disjunctiveVarSets.at(clauseIndex);
    if (disjunctiveVarSet.empty()) { // clause with no disjunctive var
      clauseGroups.push_back(vector<Int>{clauseIndex});
    }
    else { // clause with disjunctive var is put first in clauseMap then in clauseGroups
      Int var = *disjunctiveVarSet.begin(); // arbitrary member
      Int representative = varBlocks.find_set(var);
      if (clauseMap.find(representative) == clauseMap.end()) {
        clauseMap[representative] = vector<Int>{clauseIndex};
      }
      else {
        clauseMap.at(representative).push_back(clauseIndex);
      }
    }
  }

  for (const auto &kv : clauseMap) { // adds to clauseGroups clauses with disjunctive vars
    if (verbosityLevel >= 2) {
      for (Int clauseIndex : kv.second) {
        cout << "c var " << kv.first << " represents clause " << clauseIndex << "\n";
      }
    }
    clauseGroups.push_back(kv.second);
  }
}

JoinNonterminal *JoinRootBuilder::buildRoot(VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering, ClusteringHeuristic clusteringHeuristic) {
  vector<JoinTerminal *> terminals;
  for (const vector<Int> &clause : cnf.getClauses()) {
    terminals.push_back(new JoinTerminal()); // terminal index = clause index
  }

  vector<vector<JoinNode *>> leafBlocks;
  for (const vector<Int> &clauseGroup : clauseGroups) {
    vector<JoinNode *> leafBlock;
    for (Int clauseIndex : clauseGroup) {
      leafBlock.push_back(terminals.at(clauseIndex));
    }
    leafBlocks.push_back(leafBlock);
  }

  vector<JoinNode *> nonterminals;
  for (Int i = 0; i < leafBlocks.size(); i++) {
    // cout << "c building disjunctive component " << i << "\n";
    JoinComponent disjunctiveComponent(cnf, varOrderingHeuristic, inverseVarOrdering, clusteringHeuristic, leafBlocks.at(i), cnf.getAdditiveVars());
    JoinNonterminal *disjunctiveRoot = disjunctiveComponent.getComponentRoot();
    nonterminals.push_back(disjunctiveRoot);
    if (verbosityLevel >=2) cout << "c built disjunctive component " << i << "\n";
  }

  // cout << "c building additive component\n";
  JoinComponent additiveComponent(cnf, varOrderingHeuristic, inverseVarOrdering, clusteringHeuristic, nonterminals, Set<Int>());
  JoinNonterminal *additiveRoot = additiveComponent.getComponentRoot();
  if (verbosityLevel >=2) cout << "c built additive component\n";
  return additiveRoot;
}

JoinRootBuilder::JoinRootBuilder(const Cnf &cnf) {
  this->cnf = cnf;

  setDisjunctiveVarSets();
  setClauseGroups();

  if (verbosityLevel >= 2) {
    printClauseGroups();
    printDisjunctiveVarSets();
  }
}

/* class JoinTree *************************************************************/

JoinNode *JoinTree::getJoinNode(Int nodeIndex) const {
  const auto &it = joinTerminals.find(nodeIndex);
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

/* class JoinTreeProcessor ****************************************************/

Int JoinTreeProcessor::plannerPid = DUMMY_MIN_INT;

JoinNonterminal *JoinTreeProcessor::getJoinTreeRoot() const {
  return joinTree->getJoinRoot();
}

JoinTreeProcessor::JoinTreeProcessor() {
  signal(SIGINT, util::handleSignal); // Ctrl c
  signal(SIGTERM, util::handleSignal); // timeout
}

/* class JoinTreeParser *******************************************************/

void JoinTreeParser::killPlanner() {
  if (plannerPid == DUMMY_MIN_INT) {
    showWarning("plannerPid == DUMMY_MIN_INT; cannot kill");
  }
  else if (kill(plannerPid, SIGKILL) == 0) { // SIGTERM is not fast enough
    printComment("successfully killed planner process with PID " + to_string(plannerPid));
  }
  else {
    showWarning("failed to kill planner processs with PID " + to_string(plannerPid));
  }
}

void JoinTreeParser::finishParsingJoinTree() {
  if (joinTree == nullptr) {
    showError("no join tree ending on/before line " + to_string(lineIndex));
  }

  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;
  if (nonterminalCount < expectedNonterminalCount) {
    showError("missing internal nodes (" + to_string(nonterminalCount) + " found, " + to_string(expectedNonterminalCount) + " expected) before current join tree ends on line " + to_string(lineIndex));
  }
  else {
    Int maxVarCount = joinTree->getJoinRoot()->getMaxVarCount(clauses);

    printComment("after " + to_string(util::getSeconds(startTime)) + "s, finished processing first join tree (width " + to_string(maxVarCount) + " | ending on/before line " + to_string(lineIndex) + ")");

    if (verbosityLevel >= 2) {
      printThinLine();
      joinTree->printTree();
      printThinLine();
    }
  }
  joinTreeEndLineIndex = lineIndex;
  problemLineIndex = DUMMY_MIN_INT;
}

void JoinTreeParser::parseInputStream(std::istream *inputStream) {
  string line;
  while (std::getline(*inputStream, line)) {
    lineIndex++;
    std::istringstream inputStringStream(line);

    if (verbosityLevel >= 3) printComment("line " + to_string(lineIndex) + "\t" + line);

    vector<string> words;
    std::copy(std::istream_iterator<string>(inputStringStream), std::istream_iterator<string>(), std::back_inserter(words));

    Int wordCount = words.size();
    if (wordCount < 1) continue;

    const string &startWord = words.at(0);
    if (startWord == "{'':") continue; // SlurmQueen `echo` line

    if (startWord == PROBLEM_WORD) {
      if (problemLineIndex != DUMMY_MIN_INT) {
        showError("multiple problem lines: " + to_string(problemLineIndex) + " and " + to_string(lineIndex));
      }
      problemLineIndex = lineIndex;

      if (wordCount != 5) {
        showError("problem line " + to_string(lineIndex) + " has " + to_string(wordCount) + " words (should be 5)");
      }
      const string &jtWord = words.at(1);
      if (jtWord != JT_WORD) {
        showError("expected '" + JT_WORD + "', found '" + jtWord + "' | line " + to_string(lineIndex));
      }

      Int declaredVarCount = std::stoll(words.at(2));
      Int declaredClauseCount = std::stoll(words.at(3));
      Int declaredNodeCount = std::stoll(words.at(4));

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
          JoinTreeParser::plannerPid = std::stoll(value);
        }
        else if (key == "seconds") {
          if (joinTree != nullptr) {
            joinTree->plannerSeconds = std::stold(value);
            finishParsingJoinTree();
            if (inputStream == &std::cin) killPlanner();
            return;
          }
        }
      }
    }
    else { // internal-node line
      if (problemLineIndex == DUMMY_MIN_INT) {
        string message = "no problem line before internal node | line " + to_string(lineIndex);
        showError(message);
      }

      Int parentIndex = std::stoll(startWord) - 1; // 0-indexing
      if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
        showError("wrong internal-node index | line " + to_string(lineIndex));
      }

      vector<JoinNode *> children;
      Set<Int> projectableVars;
      bool parsingElimVars = false;
      for (Int i = 1; i < wordCount; i++) {
        const string &word = words.at(i);
        if (word == VAR_ELIM_WORD) {
          parsingElimVars = true;
        }
        else {
          Int num = std::stoll(word);
          if (parsingElimVars) {
            Int declaredVarCount = joinTree->declaredVarCount;
            if (num <= 0 || num > declaredVarCount) {
              showError("var '" + to_string(num) + "' inconsistent with declared var count '" + to_string(declaredVarCount) + "' | line " + to_string(lineIndex));
            }
            projectableVars.insert(num);
          }
          else {
            Int childIndex = num - 1; // 0-indexing
            if (childIndex < 0 || childIndex >= parentIndex) {
              showError("child '" + word + "' wrong | line " + to_string(lineIndex));
            }
            children.push_back(joinTree->getJoinNode(childIndex));
          }
        }
      }
      joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectableVars, parentIndex);
    }
  }

  finishParsingJoinTree();
}

JoinTreeParser::JoinTreeParser(
  const string &filePath,
  const vector<vector<Int>> &clauses
) {
  this->clauses = clauses;

  printComment("procressing join tree...", 1);

  std::ifstream inputFileStream(filePath); // variable will be destroyed if it goes out of scope
  std::istream *inputStream;
  if (filePath == STDIN_CONVENTION) {
    inputStream = &std::cin;

    printThickLine();
    printComment("getting join tree from stdin... (end input with 'Enter' then 'Ctrl d')");
  }
  else {
    if (!inputFileStream.is_open()) {
      showError("unable to open file '" + filePath + "'");
    }
    inputStream = &inputFileStream;
  }

  parseInputStream(inputStream);

  if (inputStream == &std::cin) {
    if (plannerPid != DUMMY_MIN_INT && kill(plannerPid, 0) == 0) { // null signal for error checking
      showWarning("planner should have been killed; killing it now");
      killPlanner();
    }

    printComment("getting join tree from stdin: done");
    printThickLine();
  }

  if (verbosityLevel >= 1) {
    util::printRow("declaredVarCount", joinTree->declaredVarCount);
    util::printRow("declaredClauseCount", joinTree->declaredClauseCount);
    util::printRow("declaredNodeCount", joinTree->declaredNodeCount);
    util::printRow("plannerSeconds", joinTree->plannerSeconds);
  }
}

/* class JoinTreeReader *******************************************************/

Float JoinTreeReader::timeoutSeconds;
TimePoint JoinTreeReader::startPoint;
Float JoinTreeReader::performanceFactor;

void JoinTreeReader::killPlanner() {
  if (plannerPid == DUMMY_MIN_INT) {
    showWarning("plannerPid == DUMMY_MIN_INT; cannot kill");
  }
  else if (kill(plannerPid, SIGKILL) == 0) { // SIGTERM is not fast enough
    printComment("successfully killed planner process with PID " + to_string(plannerPid));
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
  printComment("received SIGALRM after " + to_string(util::getSeconds(startTime)) + "s since main programm started");
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
  printComment("disarmed alarm");
}

void JoinTreeReader::reviseAlarm(Float newTimeoutSeconds) {
  if (performanceFactor <= 0) return;

  Float remainingSeconds = timeoutSeconds - util::getSeconds(startPoint);

  if (remainingSeconds <= 0 && !isDisarmed()) {
    util::showWarning("alarm should have been triggered " + to_string(-remainingSeconds) + "s ago; calling SIGALRM handler now");
    handleAlarm(SIGALRM);
    return;
  }

  printComment("remaining duration: " + to_string(remainingSeconds) + "s");

  cout << "c requested new timeout: " << newTimeoutSeconds << "s";
  if (newTimeoutSeconds < remainingSeconds) {
    cout << " | request granted\n";

    timeoutSeconds = newTimeoutSeconds;
    startPoint = util::getTimePoint();

    setAlarm(timeoutSeconds);
  }
  else {
    cout << " | request declined\n";
  }
}

void JoinTreeReader::initializeAlarm(Float seconds) {
  if (seconds <= 0) {
    util::showError("seconds == " + to_string(seconds) + " <= 0");
  }

  timeoutSeconds = seconds;
  startPoint = util::getTimePoint();

  signal(SIGALRM, handleAlarm); // assigns handleAlarm as the handler of SIGALRM
  setAlarm(seconds);

  printComment("constructed timer with " + to_string(seconds) + "s timeout");
}

Float JoinTreeReader::getNewTimeoutSeconds(Int maxVarCount) const {
  Float ddOperations = pow(2, maxVarCount); // big-O approximation
  Float newTimeoutSeconds = ddOperations * performanceFactor;
  return newTimeoutSeconds;
}

void JoinTreeReader::finishReadingJoinTree(bool alarming) {
  if (joinTree == nullptr) {
    showError("no join tree ending on/before line " + to_string(lineIndex));
  }

  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;
  if (nonterminalCount < expectedNonterminalCount) {
    showWarning("missing internal nodes (" + to_string(nonterminalCount) + " found, " + to_string(expectedNonterminalCount) + " expected) before current join tree ends on line " + to_string(lineIndex));

    if (joinTreeEndLineIndex == DUMMY_MIN_INT) {
      showError("no backup join tree");
    }
    else {
      showWarning("restoring backup join tree ending on line " + to_string(joinTreeEndLineIndex));
      if (verbosityLevel >= 1) {
        printThinLine();
        printComment("restored backup join tree:");
        backupJoinTree->printTree();
        printThinLine();
      }

      joinTree = backupJoinTree;
      JoinNode::restoreStaticFields();
    }
  }
  else {
    Int maxVarCount = joinTree->getJoinRoot()->getMaxVarCount(clauses);

    printComment("after " + to_string(util::getSeconds(startTime)) + "s, finished processing last complete join tree (width " + to_string(maxVarCount) + " | ending on/before line " + to_string(lineIndex) + ")");

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
  while (std::getline(*inputStream, line) && (inputStream != &std::cin || !isDisarmed())) {
    lineIndex++;
    std::istringstream inputStringStream(line);

    if (verbosityLevel >= 3) printComment("line " + to_string(lineIndex) + "\t" + line);

    vector<string> words;
    std::copy(std::istream_iterator<string>(inputStringStream), std::istream_iterator<string>(), std::back_inserter(words));

    Int wordCount = words.size();
    if (wordCount < 1) continue;

    const string &startWord = words.at(0);
    if (startWord == "{'':") continue; // SlurmQueen `echo` line

    if (startWord == "=") {
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
        showError("expected '" + JT_WORD + "', found '" + jtWord + "' | line " + to_string(lineIndex));
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
    else { // internal-node line
      if (problemLineIndex == DUMMY_MIN_INT) {
        string message = "no problem line before internal node | line " + to_string(lineIndex);
        if (joinTreeEndLineIndex != DUMMY_MIN_INT) {
          message += " (last legal join tree ends on line " + to_string(joinTreeEndLineIndex) + ")";
        }
        showError(message);
      }

      Int parentIndex = std::stoll(startWord) - 1; // 0-indexing
      if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
        showError("wrong internal-node index | line " + to_string(lineIndex));
      }

      vector<JoinNode *> children;
      Set<Int> projectableVars;
      bool readingElimVars = false;
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
              showError("var '" + to_string(num) + "' inconsistent with declared var count '" + to_string(declaredVarCount) + "' | line " + to_string(lineIndex));
            }
            projectableVars.insert(num);
          }
          else {
            Int childIndex = num - 1; // 0-indexing
            if (childIndex < 0 || childIndex >= parentIndex) {
              showError("child '" + word + "' wrong | line " + to_string(lineIndex));
            }
            children.push_back(joinTree->getJoinNode(childIndex));
          }
        }
      }
      joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectableVars, parentIndex);
    }
  }

  if (inputStream == &std::cin && !isDisarmed()) {
    showWarning("alarm should have been disarmed; disarming alarm now");
    disarmAlarm();
  }

  finishReadingJoinTree(false);
}

JoinTreeReader::JoinTreeReader(
  const string &filePath,
  Float jtWaitSeconds,
  Float performanceFactor,
  const vector<vector<Int>> &clauses
) {
  this->performanceFactor = performanceFactor;

  this->jtWaitSeconds = jtWaitSeconds;
  this->clauses = clauses;

  printComment("procressing join tree...", 1);

  std::ifstream inputFileStream(filePath); // variable will be destroyed if it goes out of scope
  std::istream *inputStream;
  if (filePath == STDIN_CONVENTION) {
    inputStream = &std::cin;
    initializeAlarm(jtWaitSeconds);

    printThickLine();
    printComment("getting join tree from stdin with " + to_string(jtWaitSeconds) + "s timeout... (end input with 'Enter' then 'Ctrl d')");
  }
  else {
    if (!inputFileStream.is_open()) {
      showError("unable to open file '" + filePath + "'");
    }
    inputStream = &inputFileStream;
  }

  readInputStream(inputStream);

  if (inputStream == &std::cin) {
    if (plannerPid != DUMMY_MIN_INT && kill(plannerPid, 0) == 0) { // null signal for error checking
      showWarning("planner should have been killed; killing it now");
      killPlanner();
    }

    printComment("getting join tree from stdin: done");
    printThickLine();
  }

  if (verbosityLevel >= 1) {
    util::printRow("declaredVarCount", joinTree->declaredVarCount);
    util::printRow("declaredClauseCount", joinTree->declaredClauseCount);
    util::printRow("declaredNodeCount", joinTree->declaredNodeCount);
    util::printRow("plannerSeconds", joinTree->plannerSeconds);
  }
}
