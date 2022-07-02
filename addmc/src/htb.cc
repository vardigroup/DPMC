#include "htb.hh"

/* classes for planning ===================================================== */

/* class JoinComponent ====================================================== */

JoinNonterminal* JoinComponent::getComponentRoot() {
  for (Int clusterIndex = 0; clusterIndex < projectableVars.size(); clusterIndex++) {
    const vector<JoinNode*>& children = nodeClusters.at(clusterIndex);
    if (!children.empty()) {
      JoinNonterminal* node = new JoinNonterminal(children, projectableVarSets.at(clusterIndex));
      Int target = node->chooseClusterIndex(clusterIndex, projectableVarSets, clusteringHeuristic);
      nodeClusters.at(target).push_back(node);
    }
  }

  return new JoinNonterminal(nodeClusters.back());
}

Set<Int> JoinComponent::getNodeVars(const vector<JoinNode*>& nodes) const {
  Set<Int> vars;
  for (JoinNode* node : nodes) {
    util::unionize(vars, node->getPostProjectionVars());
  }
  return vars;
}

vector<Int> JoinComponent::getRestrictedVarOrder() const {
  vector<Int> restrictedVarOrder;
  for (Int var : JoinNode::cnf.getCnfVarOrder(varOrderHeuristic)) {
    if (projectableVars.contains(var)) {
      restrictedVarOrder.push_back(var);
    }
  }
  return restrictedVarOrder;
}

JoinComponent::JoinComponent(Int varOrderHeuristic, string clusteringHeuristic, const vector<JoinNode*>& subtrees, const Set<Int>& keptVars) {
  this->varOrderHeuristic = varOrderHeuristic;
  this->clusteringHeuristic = clusteringHeuristic;
  this->subtrees = subtrees;
  this->keptVars = keptVars;

  projectableVars = util::getDiff(getNodeVars(subtrees), keptVars);

  nodeClusters = vector<vector<JoinNode*>>(projectableVars.size() + 1, vector<JoinNode*>());
  for (JoinNode* subtree : subtrees) {
    Int nodeRank = subtree->getNodeRank(
      getRestrictedVarOrder(), // omega
      clusteringHeuristic
    );
    nodeClusters.at(nodeRank).push_back(subtree);
  }

  projectableVarSets = vector<Set<Int>>(projectableVars.size(), Set<Int>());
  Set<Int> projectedVars; // accumulates Z_m..Z_i
  for (Int clusterIndex = projectableVars.size() - 1; clusterIndex >= 0; clusterIndex--) {
    Set<Int> projectableVarSet = util::getIntersection(projectableVars, getNodeVars(nodeClusters.at(clusterIndex)));
    projectableVarSets.at(clusterIndex) = util::getDiff(projectableVarSet, projectedVars);
    util::unionize(projectedVars, projectableVarSet);
  }
}

/* class JoinRootBuilder ==================================================== */

void JoinRootBuilder::printInnerVarSets() const {
  cout << "c inner vars: { ";
  for (Int var : util::getSortedNums(innerVars)) {
    cout << var << " ";
  }
  cout << "}\n";

  for (Int clauseIndex = 0; clauseIndex < innerVarSets.size(); clauseIndex++) {
    cout << "c inner vars of clause " << clauseIndex + 1 << ": {";
    for (Int v : util::getSortedNums(innerVarSets.at(clauseIndex))) {
      cout << " " << v;
    }
    cout << " }\n";
  }
}

void JoinRootBuilder::printClauseGroups() const {
  for (Int i = 0; i < clauseGroups.size(); i++) {
    cout << "c clause group " << i + 1 << " contains these clause indices: {";
    for (Int clauseIndex : clauseGroups.at(i)) {
      cout << " " << clauseIndex + 1;
    }
    cout << " }\n";
  }
}

void JoinRootBuilder::setInnerVarSets() {
  innerVars = JoinNode::cnf.getInnerVars();
  for (const Clause& clause : JoinNode::cnf.clauses) {
    innerVarSets.push_back(util::getIntersection(clause.getClauseVars(), innerVars));
  }
}

void JoinRootBuilder::setClauseGroups() {
  Int boostSize = JoinNode::cnf.declaredVarCount + JoinNode::cnf.clauses.size() + 10; // extra space to avoid memory errors
  vector<Int> rank(boostSize);
  vector<Int> parent(boostSize);
  boost::disjoint_sets<Int*, Int*> varBlocks(&rank.front(), &parent.front());

  for (Int var : innerVars) {
    varBlocks.make_set(var);
  }

  for (Int i = 0; i < innerVarSets.size(); i++) {
    Int element = MIN_INT;
    for (Int var : innerVarSets.at(i)) {
      if (element != MIN_INT) {
        varBlocks.union_set(var, element);
      }
      element = var;
    }
  }

  Map<Int, vector<Int>> clauseMap; // representative var |-> clause indices

  for (Int clauseIndex = 0; clauseIndex < innerVarSets.size(); clauseIndex++) {
    const Set<Int>& innerVarSet = innerVarSets.at(clauseIndex);
    if (innerVarSet.empty()) { // clause with no inner var
      clauseGroups.push_back({clauseIndex});
    }
    else { // clause with inner var is put first in `clauseMap` then in `clauseGroups`
      Int var = *innerVarSet.begin(); // arbitrary member
      Int representative = varBlocks.find_set(var);
      if (clauseMap.contains(representative)) {
        clauseMap.at(representative).push_back(clauseIndex);
      }
      else {
        clauseMap[representative] = {clauseIndex};
      }
    }
  }

  for (const pair<Int, vector<Int>>& kv : clauseMap) { // adds to `clauseGroups` clauses with inner vars
    if (verboseSolving >= 2) {
      for (Int clauseIndex : kv.second) {
        cout << "c var " << kv.first << " represents clause " << clauseIndex + 1 << "\n";
      }
    }
    clauseGroups.push_back(kv.second);
  }
}

JoinNonterminal* JoinRootBuilder::buildRoot(Int varOrderHeuristic, string clusteringHeuristic) const {
  vector<JoinTerminal*> terminals;
  for (const Clause& clause : JoinNode::cnf.clauses) {
    terminals.push_back(new JoinTerminal()); // terminal index = clause index
  }

  vector<vector<JoinNode*>> leafBlocks;
  for (const vector<Int>& clauseGroup : clauseGroups) {
    vector<JoinNode*> leafBlock;
    for (Int clauseIndex : clauseGroup) {
      leafBlock.push_back(terminals.at(clauseIndex));
    }
    leafBlocks.push_back(leafBlock);
  }

  vector<JoinNode*> nonterminals;
  for (Int i = 0; i < leafBlocks.size(); i++) {
    if (verboseSolving >= 2) {
      cout << "c building inner component " << i + 1 << ": started\n";
    }
    JoinComponent innerComponent(varOrderHeuristic, clusteringHeuristic, leafBlocks.at(i), JoinNode::cnf.outerVars);
    JoinNonterminal* innerRoot = innerComponent.getComponentRoot();
    nonterminals.push_back(innerRoot);
    if (verboseSolving >= 2) {
      cout << "c building inner component " << i + 1 << ": ended\n";
    }
  }

  if (verboseSolving >= 2) {
    cout << "c building outer component: started\n";
  }
  JoinComponent outerComponent(varOrderHeuristic, clusteringHeuristic, nonterminals, Set<Int>());
  JoinNonterminal* outerRoot = outerComponent.getComponentRoot();
  if (verboseSolving >= 2) {
    cout << "c building outer component: ended\n";
  }

  return outerRoot;
}

JoinRootBuilder::JoinRootBuilder() {
  setInnerVarSets();
  setClauseGroups();

  if (verboseSolving >= 2) {
    printClauseGroups();
    printInnerVarSets();
  }
}

/* class Planner ============================================================ */

void Planner::printJoinTree() const {
  cout << "p " << JOIN_TREE_WORD << " " << JoinNode::cnf.declaredVarCount << " " << joinRoot->terminalCount << " " << joinRoot->nodeCount << "\n";
  joinRoot->printSubtree();
}

void Planner::outputJoinTree() {
  cout << "c computing output...\n";

  setJoinTree();

  cout << DASH_LINE;
  printJoinTree();
  cout << DASH_LINE;

  printRow("joinTreeWidth", joinRoot->getWidth());
}

/* class BucketElimPlanner ================================================== */

void BucketElimPlanner::setJoinTree() {
  joinRoot = JoinRootBuilder().buildRoot(clusterVarOrderHeuristic, treeClustering ? BUCKET_ELIM_TREE : BUCKET_ELIM_LIST);
}

BucketElimPlanner::BucketElimPlanner(bool treeClustering, Int clusterVarOrderHeuristic) {
  this->treeClustering = treeClustering;
  this->clusterVarOrderHeuristic = clusterVarOrderHeuristic;
}

/* class BouquetMethodPlanner =============================================== */

void BouquetMethodPlanner::setJoinTree() {
  joinRoot = JoinRootBuilder().buildRoot(clusterVarOrderHeuristic, treeClustering ? BOUQUET_METHOD_TREE : BOUQUET_METHOD_LIST);
}

BouquetMethodPlanner::BouquetMethodPlanner(bool treeClustering, Int clusterVarOrderHeuristic) {
  this->treeClustering = treeClustering;
  this->clusterVarOrderHeuristic = clusterVarOrderHeuristic;
}

/* class OptionDict ========================================================= */

string OptionDict::helpClusterVarOrderHeuristic() {
  return "cluster var order" + util::helpVarOrderHeuristic(CNF_VAR_ORDER_HEURISTICS);
}

string OptionDict::helpClusteringHeuristic() {
  string s = "clustering heuristic: ";
  for (auto it = CLUSTERING_HEURISTICS.begin(); it != CLUSTERING_HEURISTICS.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != CLUSTERING_HEURISTICS.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

void OptionDict::runCommand() const {
  if (verboseSolving >= 1) {
    cout << "c processing command-line options...\n";
    printRow("cnfFile", cnfFilePath);
    printRow("projectedCounting", projectedCounting);
    printRow("randomSeed", randomSeed);
    printRow("clusterVarOrderHeuristic", (clusterVarOrderHeuristic < 0 ? "INVERSE_" : "") + CNF_VAR_ORDER_HEURISTICS.at(abs(clusterVarOrderHeuristic)));
    printRow("clusteringHeuristic", CLUSTERING_HEURISTICS.at(clusteringHeuristic));
    cout << "\n";
  }

  try {
    JoinNode::cnf.readCnfFile(cnfFilePath);
    if (clusteringHeuristic == BUCKET_ELIM_LIST) {
      BucketElimPlanner bucketElimPlanner(false, clusterVarOrderHeuristic);
      bucketElimPlanner.outputJoinTree();
    }
    else if (clusteringHeuristic == BUCKET_ELIM_TREE) {
      BucketElimPlanner bucketElimPlanner(true, clusterVarOrderHeuristic);
      bucketElimPlanner.outputJoinTree();
    }
    else if (clusteringHeuristic == BOUQUET_METHOD_LIST) {
      BouquetMethodPlanner bouquetMethodPlanner(false, clusterVarOrderHeuristic);
      bouquetMethodPlanner.outputJoinTree();
    }
    else {
      assert(clusteringHeuristic == BOUQUET_METHOD_TREE);
      BouquetMethodPlanner bouquetMethodPlanner(true, clusterVarOrderHeuristic);
      bouquetMethodPlanner.outputJoinTree();
    }
  }
  catch (EmptyClauseException) {}
}

OptionDict::OptionDict(int argc, char** argv) {
  cxxopts::Options options("htb", "Heuristic Tree Builder");
  options.set_width(118);

  using cxxopts::value;
  options.add_options()
    (CNF_FILE_OPTION, "CNF file path; string (required)", value<string>())
    (PROJECTED_COUNTING_OPTION, "projected counting (graded join tree): 0, 1; int", value<Int>()->default_value("0"))
    (RANDOM_SEED_OPTION, "random seed; int", value<Int>()->default_value("0"))
    (CLUSTER_VAR_OPTION, helpClusterVarOrderHeuristic(), value<Int>()->default_value(to_string(LEX_P_HEURISTIC)))
    (CLUSTERING_HEURISTIC_OPTION, helpClusteringHeuristic(), value<string>()->default_value(BOUQUET_METHOD_TREE))
    (VERBOSE_CNF_OPTION, util::helpVerboseCnfProcessing(), value<Int>()->default_value("0"))
    (VERBOSE_SOLVING_OPTION, util::helpVerboseSolving(), value<Int>()->default_value("0"))
    (HELP_OPTION, "help")
  ;

  cxxopts::ParseResult result = options.parse(argc, argv);
  if (result.count(HELP_OPTION) || !result.count(CNF_FILE_OPTION)) {
    cout << options.help();
  }
  else {
    cout << "c htb process:\n";
    cout << "c pid " << getpid() << "\n\n";

    cnfFilePath = result[CNF_FILE_OPTION].as<string>();

    projectedCounting = result[PROJECTED_COUNTING_OPTION].as<Int>(); // global var

    randomSeed = result[RANDOM_SEED_OPTION].as<Int>(); // global var

    clusterVarOrderHeuristic = result[CLUSTER_VAR_OPTION].as<Int>();
    assert(CNF_VAR_ORDER_HEURISTICS.contains(abs(clusterVarOrderHeuristic)));

    clusteringHeuristic = result[CLUSTERING_HEURISTIC_OPTION].as<string>();
    assert(CLUSTERING_HEURISTICS.contains(clusteringHeuristic));

    verboseCnf = result[VERBOSE_CNF_OPTION].as<Int>(); // global var

    verboseSolving = result[VERBOSE_SOLVING_OPTION].as<Int>(); // global var

    toolStartPoint = util::getTimePoint(); // global var
    runCommand();
    printRow("seconds", util::getDuration(toolStartPoint));
  }
}

/* global functions ========================================================= */

int main(int argc, char** argv) {
  cout << std::unitbuf; // enables automatic flushing
  OptionDict(argc, argv);
}
