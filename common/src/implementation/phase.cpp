/* inclusions *****************************************************************/

#include "../interface/phase.hpp"

/* classes ********************************************************************/

/* class Tool *****************************************************************/

void Tool::printJoinTree(const Cnf &cnf) const {
  cout << PROBLEM_WORD << " " << JT_WORD << " " << cnf.getDeclaredVarCount() << " " << joinRoot->getTerminalCount() << " " << joinRoot->getNodeCount() << "\n";
  joinRoot->printSubtree();
  cout << "c joinTreeWidth " << joinRoot->getMaxVarCount(cnf.getClauses()) << "\n";
}

/* class Planner **************************************************************/

void Planner::setJoinTree(const Cnf &cnf) {
  if (cnf.getClauses().empty()) { // empty cnf
    showWarning("cnf is empty");
    joinRoot = new JoinNonterminal(vector<JoinNode *>());
    return;
  }

  Int i = cnf.getEmptyClauseIndex();
  if (i == DUMMY_MIN_INT) {
    constructJoinTree(cnf);
  }
  else {
    showWarning("clause " + to_string(i + 1) + " of cnf is empty (1-indexing); generating dummy join tree");
    joinRoot = new JoinNonterminal(vector<JoinNode *>());
  }
}

JoinNonterminal *Planner::getJoinTree(const Cnf &cnf) {
  if (joinRoot == nullptr) {
    setJoinTree(cnf);
  }
  return joinRoot;
}

void Planner::outputJoinTree(const Cnf &cnf) {
  printComment("computing output...", 1);

  signal(SIGINT, util::handleSignal); // Ctrl c
  signal(SIGTERM, util::handleSignal); // timeout

  setJoinTree(cnf);
  printThinLine();
  printJoinTree(cnf);
  printThinLine();
}

/* class BucketPlanner ********************************************************/

void BucketPlanner::constructJoinTree(const Cnf &cnf) {
  joinRoot = JoinRootBuilder(cnf).buildRoot(cnfVarOrderingHeuristic, inverseCnfVarOrdering, (usingTreeClustering ? ClusteringHeuristic::BUCKET_TREE : ClusteringHeuristic::BUCKET_LIST));
}

BucketPlanner::BucketPlanner(bool usingTreeClustering, VarOrderingHeuristic cnfVarOrderingHeuristic, bool inverseCnfVarOrdering) {
  this->usingTreeClustering = usingTreeClustering;
  this->cnfVarOrderingHeuristic = cnfVarOrderingHeuristic;
  this->inverseCnfVarOrdering = inverseCnfVarOrdering;
}

/* class BouquetPlanner *******************************************************/

void BouquetPlanner::constructJoinTree(const Cnf &cnf) {
  joinRoot = JoinRootBuilder(cnf).buildRoot(cnfVarOrderingHeuristic, inverseCnfVarOrdering, (usingTreeClustering ? ClusteringHeuristic::BOUQUET_TREE : ClusteringHeuristic::BOUQUET_LIST));
}

BouquetPlanner::BouquetPlanner(bool usingTreeClustering, VarOrderingHeuristic cnfVarOrderingHeuristic, bool inverseCnfVarOrdering) {
  this->usingTreeClustering = usingTreeClustering;
  this->cnfVarOrderingHeuristic = cnfVarOrderingHeuristic;
  this->inverseCnfVarOrdering = inverseCnfVarOrdering;
}

/* class Executor *************************************************************/

void Executor::orderDdVars(const Cnf &cnf) {
  ddVarToCnfVarMap = cnf.getVarOrdering(ddVarOrderingHeuristic, inverseDdVarOrdering);
  for (Int ddVar = 0; ddVar < ddVarToCnfVarMap.size(); ddVar++) {
    Int cnfVar = ddVarToCnfVarMap.at(ddVar);
    cnfVarToDdVarMap[cnfVar] = ddVar;
  }
}

Dd Executor::getClauseDd(const vector<Int> &clause) const {
  Dd clauseDd = Dd::getZeroDd();
  for (Int literal : clause) {
    Int cnfVar = util::getCnfVar(literal);
    Int ddVar = cnfVarToDdVarMap.at(cnfVar);
    Dd literalDd = util::isPositiveLiteral(literal) ? Dd::getVarDd(ddVar) : Dd::getNegativeLiteralDd(ddVar);
    clauseDd = clauseDd.getDisjunction(literalDd);
  }
  return clauseDd;
}

Dd Executor::countSubtree(JoinNode *joinNode, const Cnf &cnf, Set<Int> &projectedCnfVars) {
  if (joinNode->isTerminal()) {
    return getClauseDd(cnf.getClauses().at(joinNode->getNodeIndex()));
  }
  else {
    Dd dd = Dd::getOneDd();
    switch (joinPriority) {
      case JoinPriority::ARBITRARY: { // arbitrarily multiplies child ADDs
        for (JoinNode *child : joinNode->getChildren()) {
          dd = dd.getProduct(countSubtree(child, cnf, projectedCnfVars));
        }
        break;
      }
      case JoinPriority::SMALLEST_FIRST: {} // falls through because Dd::operator< handles both SMALLEST_FIRST and LARGEST_FIRST
      case JoinPriority::LARGEST_FIRST: {
        std::priority_queue<Dd, vector<Dd>> childDds;

        for (JoinNode *child : joinNode->getChildren()) {
          childDds.push(countSubtree(child, cnf, projectedCnfVars));
        }

        bool testing = false;
        // testing = true;
        if (testing) {
          Dd::testPriorityQueue(childDds);
          util::showError("printed and cleared childDds");
        }

        if (childDds.empty()) showError("no child ADD");
        while (childDds.size() > 1) {
          Dd dd1 = childDds.top();
          childDds.pop();

          Dd dd2 = childDds.top();
          childDds.pop();

          Dd dd3 = dd1.getProduct(dd2);
          childDds.push(dd3);
        }
        dd = childDds.top();
        break;
      }
      default: { util::showError("illegal joinPriority"); }
    }

    const Set<Int> &additiveVars = cnf.getAdditiveVars();
    for (Int cnfVar : joinNode->getProjectableVars()) {
      projectedCnfVars.insert(cnfVar);

      Int ddVar = cnfVarToDdVarMap.at(cnfVar);
      bool additive = additiveVars.find(ddVarToCnfVarMap.at(ddVar)) != additiveVars.end();
      dd = dd.getAbstraction(ddVar, ddVarToCnfVarMap, cnf.getLiteralWeights(), additive);
    }
    return dd;
  }
}

Float Executor::computeModelCount(const Cnf &cnf) {
  orderDdVars(cnf);

  Set<Int> projectedCnfVars;
  Dd dd = countSubtree(static_cast<JoinNode *>(joinRoot), cnf, projectedCnfVars);

  Float modelCount = dd.countConstDdFloat();
  modelCount = util::adjustModelCount(modelCount, projectedCnfVars, cnf.getLiteralWeights());
  return modelCount;
}

Float Executor::getModelCount(const Cnf &cnf) {
  Int i = cnf.getEmptyClauseIndex();
  if (i != DUMMY_MIN_INT) { // empty clause found
    showWarning("clause " + to_string(i + 1) + " of cnf is empty (1-indexing)");
    return 0;
  }
  else {
    return computeModelCount(cnf);
  }
}

void Executor::outputModelCount(const Cnf &cnf) {
  printComment("computing output...", 1);

  signal(SIGINT, util::handleSignal); // Ctrl c
  signal(SIGTERM, util::handleSignal); // timeout

  util::printSolutionLine(getModelCount(cnf));
}

Executor::Executor(
  JoinNonterminal *joinRoot,
  VarOrderingHeuristic ddVarOrderingHeuristic,
  bool inverseDdVarOrdering
) {
  this->joinRoot = joinRoot;
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;
}
