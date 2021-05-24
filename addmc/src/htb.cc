#include "htb.hh"

/* classes ================================================================== */

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

void JoinRootBuilder::printDisjunctiveVarSets() const {
  cout << "c disjunctiveVars: { ";
  for (Int var : disjunctiveVars) {
    cout << var << " ";
  }
  cout << "}\n";

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
    cout << "c clause group " << i << " contains these clause indices:";
    for (Int clauseIndex : clauseGroups.at(i)) {
      cout << " " << clauseIndex;
    }
    cout << "\n";
  }
}

void JoinRootBuilder::setDisjunctiveVarSets() {
  disjunctiveVars = JoinNode::cnf.getDisjunctiveVars();
  for (const Clause& clause : JoinNode::cnf.clauses) {
    disjunctiveVarSets.push_back(util::getIntersection(clause.getClauseVars(), disjunctiveVars));
  }
}

void JoinRootBuilder::setClauseGroups() {
  Int boostSize = JoinNode::cnf.declaredVarCount + JoinNode::cnf.clauses.size() + 10; // extra space to avoid memory errors
  vector<Int> rank(boostSize);
  vector<Int> parent(boostSize);
  boost::disjoint_sets<Int*, Int*> varBlocks(&rank.front(), &parent.front());

  for (Int var : disjunctiveVars) {
    varBlocks.make_set(var);
  }

  for (Int i = 0; i < disjunctiveVarSets.size(); i++) {
    Int element = MIN_INT;
    for (Int var : disjunctiveVarSets.at(i)) {
      if (element != MIN_INT) {
        varBlocks.union_set(var, element);
      }
      element = var;
    }
  }

  Map<Int, vector<Int>> clauseMap; // representative var |-> clause indices

  for (Int clauseIndex = 0; clauseIndex < disjunctiveVarSets.size(); clauseIndex++) {
    const Set<Int>& disjunctiveVarSet = disjunctiveVarSets.at(clauseIndex);
    if (disjunctiveVarSet.empty()) { // clause with no disjunctive var
      clauseGroups.push_back(vector<Int>{clauseIndex});
    }
    else { // clause with disjunctive var is put first in clauseMap then in clauseGroups
      Int var = *disjunctiveVarSet.begin(); // arbitrary member
      Int representative = varBlocks.find_set(var);
      if (clauseMap.contains(representative)) {
        clauseMap.at(representative).push_back(clauseIndex);
      }
      else {
        clauseMap[representative] = vector<Int>{clauseIndex};
      }
    }
  }

  for (const pair<Int, vector<Int>>& kv : clauseMap) { // adds to clauseGroups clauses with disjunctive vars
    if (verboseSolving >= 2) {
      for (Int clauseIndex : kv.second) {
        cout << "c var " << kv.first << " represents clause " << clauseIndex << "\n";
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
      cout << "c building disjunctive component " << i << "\n";
    }
    JoinComponent disjunctiveComponent(varOrderHeuristic, clusteringHeuristic, leafBlocks.at(i), JoinNode::cnf.additiveVars);
    JoinNonterminal* disjunctiveRoot = disjunctiveComponent.getComponentRoot();
    nonterminals.push_back(disjunctiveRoot);
    if (verboseSolving >= 2) {
      cout << "c building disjunctive component " << i << ": done\n";
    }
  }

  if (verboseSolving >= 2) {
    cout << "c building additive component\n";
  }
  JoinComponent additiveComponent(varOrderHeuristic, clusteringHeuristic, nonterminals, Set<Int>());
  JoinNonterminal* additiveRoot = additiveComponent.getComponentRoot();
  if (verboseSolving >= 2) {
    cout << "c building additive component: done\n";
  }

  return additiveRoot;
}

JoinRootBuilder::JoinRootBuilder() {
  setDisjunctiveVarSets();
  setClauseGroups();

  if (verboseSolving >= 2) {
    printClauseGroups();
    printDisjunctiveVarSets();
  }
}

/* class Planner ============================================================ */

void Planner::printJoinTree() const {
  cout << "p " << JT_WORD << " " << JoinNode::cnf.declaredVarCount << " " << joinRoot->terminalCount << " " << joinRoot->nodeCount << "\n";
  joinRoot->printSubtree();
}

void Planner::outputJoinTree() {
  cout << "c computing output...\n";

  setJoinTree();

  cout << THIN_LINE;
  printJoinTree();
  cout << THIN_LINE;

  util::printRow("joinTreeWidth", joinRoot->getWidth());
}

/* class BucketPlanner ====================================================== */

void BucketPlanner::setJoinTree() {
  joinRoot = JoinRootBuilder().buildRoot(clusterVarOrderHeuristic, usingTreeClustering ? BUCKET_TREE : BUCKET_LIST);
}

BucketPlanner::BucketPlanner(bool usingTreeClustering, Int clusterVarOrderHeuristic) {
  this->usingTreeClustering = usingTreeClustering;
  this->clusterVarOrderHeuristic = clusterVarOrderHeuristic;
}

/* class BouquetPlanner ===================================================== */

void BouquetPlanner::setJoinTree() {
  joinRoot = JoinRootBuilder().buildRoot(clusterVarOrderHeuristic, usingTreeClustering ? BOUQUET_TREE : BOUQUET_LIST);
}

BouquetPlanner::BouquetPlanner(bool usingTreeClustering, Int clusterVarOrderHeuristic) {
  this->usingTreeClustering = usingTreeClustering;
  this->clusterVarOrderHeuristic = clusterVarOrderHeuristic;
}

/* class OptionDict ========================================================= */

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

    util::printRow("cnfFile", cnfFilePath);
    util::printRow("projectedCounting", projectedCounting);
    util::printRow("randomSeed", randomSeed);
    util::printRow("clusterVarOrder", (clusterVarOrderHeuristic < 0 ? "INVERSE_" : "") + CNF_VAR_ORDER_HEURISTICS.at(abs(clusterVarOrderHeuristic)));
    util::printRow("clusteringHeuristic", CLUSTERING_HEURISTICS.at(clusteringHeuristic));
    cout << "\n";
  }

  try {
    JoinNode::cnf = Cnf(cnfFilePath);
    if (clusteringHeuristic == BUCKET_LIST) {
      BucketPlanner bucketPlanner(false, clusterVarOrderHeuristic);
      bucketPlanner.outputJoinTree();
    }
    else if (clusteringHeuristic == BUCKET_TREE) {
      BucketPlanner bucketPlanner(true, clusterVarOrderHeuristic);
      bucketPlanner.outputJoinTree();
    }
    else if (clusteringHeuristic == BOUQUET_LIST) {
      BouquetPlanner bouquetPlanner(false, clusterVarOrderHeuristic);
      bouquetPlanner.outputJoinTree();
    }
    else {
      assert(clusteringHeuristic == BOUQUET_TREE);
      BouquetPlanner bouquetPlanner(true, clusterVarOrderHeuristic);
      bouquetPlanner.outputJoinTree();
    }
  }
  catch (EmptyClauseException) {}
}

OptionDict::OptionDict(int argc, char** argv) {
  cxxopts::Options options("htb", "Heuristic Tree Builder");
  options.set_width(105);
  options.add_options()
    (CNF_FILE_OPTION, "cnf file path; string (REQUIRED)", value<string>())
    (PROJECTED_COUNTING_OPTION, "projected counting: 0, 1; int", value<Int>()->default_value("0"))
    (RANDOM_SEED_OPTION, "random seed; int", value<Int>()->default_value("0"))
    (CLUSTER_VAR_OPTION, util::helpVarOrderHeuristic("cluster"), value<Int>()->default_value(to_string(LEXP)))
    (CLUSTERING_HEURISTIC_OPTION, helpClusteringHeuristic(), value<string>()->default_value(BOUQUET_TREE))
    (VERBOSE_CNF_OPTION, "verbose cnf: 0, " + INPUT_VERBOSITIES, value<Int>()->default_value("0"))
    (VERBOSE_SOLVING_OPTION, util::helpVerboseSolving(), value<Int>()->default_value("1"))
  ;
  cxxopts::ParseResult result = options.parse(argc, argv);
  if (result.count(CNF_FILE_OPTION)) {
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
    util::printRow("seconds", util::getDuration(toolStartPoint));
  }
  else {
    cout << options.help();
  }
}

/* global functions ========================================================= */

int main(int argc, char** argv) {
  cout << std::unitbuf; // enables automatic flushing
  OptionDict(argc, argv);
}
