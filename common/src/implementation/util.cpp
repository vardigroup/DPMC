/* inclusions *****************************************************************/

#include "../interface/util.hpp"

/* global variables ***********************************************************/

PlanningStrategy planningStrategy;
DdPackage ddPackage;
JoinPriority joinPriority;
Int dotFileIndex = 1;
Int randomSeed;
Int verbosityLevel;
TimePoint startTime;

/* constants ******************************************************************/

const string &BOLD_LINE = "******************************************************************";
const string &THICK_LINE = "==================================================================";
const string &THIN_LINE = "------------------------------------------------------------------";

const string &PROBLEM_WORD = "p";

const string &STDIN_CONVENTION = "-";

const string &REQUIRED_OPTION_GROUP = "Required";
const string &OPTIONAL_OPTION_GROUP = "Optional";

const string &HELP_OPTION = "h, hi";
const string &CNF_FILE_OPTION = "cf";
const string &WEIGHT_FORMAT_OPTION = "wf";
const string &OUTPUT_WEIGHT_FORMAT_OPTION = "ow";
const string &JT_FILE_OPTION = "jf";
const string &PLANNING_STRATEGY_OPTION = "ps";
const string &JT_WAIT_DURAION_OPTION = "jw";
const string &PERFORMANCE_FACTOR_OPTION = "pf";
const string &OUTPUT_FORMAT_OPTION = "of";
const string &CLUSTERING_HEURISTIC_OPTION = "ch";
const string &CLUSTER_VAR_ORDER_OPTION = "cv";
const string &DD_VAR_ORDER_OPTION = "dv";
const string &DD_PACKAGE_OPTION = "dp";
const string &WORKER_COUNT_OPTION = "sw"; // Sylvan workers
const string &JOIN_PRIORITY_OPTION = "jp";
const string &RANDOM_SEED_OPTION = "rs";
const string &VERBOSITY_LEVEL_OPTION = "vl";

const std::map<Int, WeightFormat> WEIGHT_FORMAT_CHOICES = {
  {1, WeightFormat::UNWEIGHTED},
  {2, WeightFormat::MINIC2D},
  {3, WeightFormat::CACHET},
  {4, WeightFormat::WCNF},
  {5, WeightFormat::WPCNF}
};
const Int DEFAULT_WEIGHT_FORMAT_CHOICE = 5;

const std::map<string, PlanningStrategy> PLANNING_STRATEGY_CHOICES = {
  {"f", PlanningStrategy::FIRST_JOIN_TREE},
  {"t", PlanningStrategy::TIMING}
};
const string DEFAULT_PLANNING_STRATEGY_CHOICE = "f";

const Float DEFAULT_JT_WAIT_SECONDS = 2.3;
const Float DEFAULT_PERFORMANCE_FACTOR = 0; // watch out for cxxopts underflow

const std::map<Int, ClusteringHeuristic> CLUSTERING_HEURISTIC_CHOICES = {
  {3, ClusteringHeuristic::BUCKET_LIST},
  {4, ClusteringHeuristic::BUCKET_TREE},
  {5, ClusteringHeuristic::BOUQUET_LIST},
  {6, ClusteringHeuristic::BOUQUET_TREE}
};
const Int DEFAULT_CLUSTERING_HEURISTIC_CHOICE = 6;

const std::map<Int, VarOrderingHeuristic> VAR_ORDERING_HEURISTIC_CHOICES = {
  {1, VarOrderingHeuristic::APPEARANCE},
  {2, VarOrderingHeuristic::DECLARATION},
  {3, VarOrderingHeuristic::RANDOM},
  {4, VarOrderingHeuristic::MCS},
  {5, VarOrderingHeuristic::LEXP},
  {6, VarOrderingHeuristic::LEXM},
  {7, VarOrderingHeuristic::MINFILL}
};
const Int DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE = 5;
const Int DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE = 4;

const std::map<Int, DdPackage> DD_PACKAGE_CHOICES = {
  {1, DdPackage::CUDD},
  {2, DdPackage::SYLVAN}
};
const Int DEFAULT_DD_PACKAGE_CHOICE = 1;

const Int DEFAULT_WORKER_COUNT = 0;

const std::map<string, JoinPriority> JOIN_PRIORITY_CHOICES = {
  {"a", JoinPriority::ARBITRARY},
  {"s", JoinPriority::SMALLEST_FIRST},
  {"l", JoinPriority::LARGEST_FIRST}
};
const string DEFAULT_JOIN_PRIORITY_CHOICE = "s";

const Int DEFAULT_RANDOM_SEED = 2020;

const vector<Int> VERBOSITY_LEVEL_CHOICES = {0, 1, 2, 3};
const Int DEFAULT_VERBOSITY_LEVEL_CHOICE = 1;

const Float NEGATIVE_INFINITY = -std::numeric_limits<Float>::infinity();

const Int DUMMY_MIN_INT = std::numeric_limits<Int>::min();
const Int DUMMY_MAX_INT = std::numeric_limits<Int>::max();

const string &DOT_DIR = "src/";

/* namespaces *****************************************************************/

/* namespace util *************************************************************/

bool util::isInt(Float f) {
  Float intPart;
  Float fractionalPart = modf(f, &intPart);
  return fractionalPart == 0;
}

/* namespaced functions: printing *********************************************/

void util::printComment(const string &message, Int preceedingNewLines, Int followingNewLines, bool commented) {
  for (Int i = 0; i < preceedingNewLines; i++) cout << "\n";
  cout << (commented ? "c " : "") << message;
  for (Int i = 0; i < followingNewLines; i++) cout << "\n";
}

void util::printSolutionLine(Float modelCount, Int preceedingThinLines, Int followingThinLines) {
  for (Int i = 0; i < preceedingThinLines; i++) printThinLine();
  cout << "s wmc " << modelCount << "\n";
  for (Int i = 0; i < followingThinLines; i++) printThinLine();
}

void util::printBoldLine(bool commented) {
  printComment(BOLD_LINE, 0, 1, commented);
}

void util::printThickLine(bool commented) {
  printComment(THICK_LINE, 0, 1, commented);
}

void util::printThinLine(std::ostream &outputStream) {
  outputStream << "c " << THIN_LINE << "\n";
}

void util::printHelpOption() {
  cout << "  -h, --hi      help information\n";
}

void util::printCnfFileOption() {
  cout << "      --" << CNF_FILE_OPTION << std::left << std::setw(56) << "=arg  cnf file path (to use stdin, type: --" + CNF_FILE_OPTION + "=-)";
  cout << "Default arg: -\n";
}

void util::printWeightFormatOption() {
  cout << "      --" << WEIGHT_FORMAT_OPTION << "=arg  ";
  cout << "weight format in cnf file:\n";
  for (const auto &kv : WEIGHT_FORMAT_CHOICES) {
    Int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getWeightFormatName(kv.second);
    if (num == DEFAULT_WEIGHT_FORMAT_CHOICE) cout << "Default arg: " << DEFAULT_WEIGHT_FORMAT_CHOICE;
    cout << "\n";
  }
}

void util::printOutputWeightFormatOption() {
  cout << "      --" << OUTPUT_WEIGHT_FORMAT_OPTION << "=arg  ";
  cout << "output weight format:\n";
  for (const auto &kv : WEIGHT_FORMAT_CHOICES) {
    Int num = kv.first;

    const auto &weightFormat = kv.second;
    if (weightFormat == WeightFormat::UNWEIGHTED);

    cout << "           " << num << "    " << std::left << std::setw(50) << getWeightFormatName(weightFormat);
    if (num == DEFAULT_WEIGHT_FORMAT_CHOICE) cout << "Default arg: " << DEFAULT_WEIGHT_FORMAT_CHOICE;
    cout << "\n";
  }
}

void util::printJtFileOption() {
  cout << "      --" << JT_FILE_OPTION << std::left << std::setw(56) << "=arg  jt file path (to use stdin, type: --" + JT_FILE_OPTION + "=-)";
  cout << "Default arg: (empty)\n";
}

void util::printPlanningStrategyOption() {
  cout << "      --" << PLANNING_STRATEGY_OPTION << "=arg  ";
  cout << "planning strategy:\n";
  for (const auto &kv : PLANNING_STRATEGY_CHOICES) {
    const string &key = kv.first;
    cout << "           " << key << "    " << std::left << std::setw(50) << getPlanningStrategyName(kv.second);
    if (key == DEFAULT_PLANNING_STRATEGY_CHOICE) cout << "Default arg: " << DEFAULT_PLANNING_STRATEGY_CHOICE;
    cout << "\n";
  }
}

void util::printJtWaitOption() {
  cout << "      --" << JT_WAIT_DURAION_OPTION << std::left << std::setw(56) << "=arg  jt wait duration before jt planner is killed";
  cout << "Default arg: " << DEFAULT_JT_WAIT_SECONDS << " (seconds)\n";
}

void util::printPerformanceFactorOption() {
  cout << "      --" << PERFORMANCE_FACTOR_OPTION << std::left << std::setw(56) << "=arg  performance factor (enable with positive float)";
  cout << "Default arg: " << DEFAULT_PERFORMANCE_FACTOR << "\n";
}

void util::printClusteringHeuristicOption() {
  cout << "      --" << CLUSTERING_HEURISTIC_OPTION << "=arg  ";
  cout << "clustering heuristic:\n";
  for (const auto &kv : CLUSTERING_HEURISTIC_CHOICES) {
    Int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getClusteringHeuristicName(kv.second);
    if (num == DEFAULT_CLUSTERING_HEURISTIC_CHOICE) cout << "Default arg: " << DEFAULT_CLUSTERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printCnfVarOrderingHeuristicOption() {
  cout << "      --" << CLUSTER_VAR_ORDER_OPTION << "=arg  ";
  cout << "cluster var order heuristic (negate to invert):\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_CHOICES) {
    Int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getVarOrderingHeuristicName(kv.second);
    if (num == abs(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE)) cout << "Default arg: +" << DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printDdVarOrderingHeuristicOption() {
  cout << "      --" << DD_VAR_ORDER_OPTION << "=arg  ";
  cout << "diagram var order heuristic (negate to invert):\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_CHOICES) {
    Int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getVarOrderingHeuristicName(kv.second);
    if (num == abs(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)) cout << "Default arg: +" << DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printDdPackageOption() {
  cout << "      --" << DD_PACKAGE_OPTION << "=arg  ";
  cout << "diagram package:\n";
  for (const auto &kv : DD_PACKAGE_CHOICES) {
    Int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getDdPackageName(kv.second);
    if (num == DEFAULT_DD_PACKAGE_CHOICE) cout << "Default arg: " << DEFAULT_DD_PACKAGE_CHOICE;
    cout << "\n";
  }
}

void util::printWorkerCountOption() {
  cout << "      --" << WORKER_COUNT_OPTION << std::left << std::setw(56) << "=arg  Sylvan workers (threads)";
  cout << "Default arg: " << DEFAULT_WORKER_COUNT << " (auto-detected)\n";
}

void util::printJoinPriorityOption() {
  cout << "      --" << JOIN_PRIORITY_OPTION << "=arg  ";
  cout << "join priority:\n";
  for (const auto &kv : JOIN_PRIORITY_CHOICES) {
    const string &key = kv.first;
    cout << "           " << key << "    " << std::left << std::setw(50) << getJoinPriorityName(kv.second);
    if (key == DEFAULT_JOIN_PRIORITY_CHOICE) cout << "Default arg: " << DEFAULT_JOIN_PRIORITY_CHOICE;
    cout << "\n";
  }
}

void util::printRandomSeedOption() {
  cout << "      --" << RANDOM_SEED_OPTION << std::left << std::setw(56) << "=arg  random seed";
  cout << "Default arg: " << DEFAULT_RANDOM_SEED << "\n";
}

void util::printVerbosityLevelOption() {
  cout << "      --" << VERBOSITY_LEVEL_OPTION << "=arg  ";
  cout << "verbosity level:\n";
  for (Int verbosityLevel : VERBOSITY_LEVEL_CHOICES) {
    cout << "           " << verbosityLevel << "    " << std::left << std::setw(50) << getVerbosityLevelName(verbosityLevel);
    if (verbosityLevel == DEFAULT_VERBOSITY_LEVEL_CHOICE) cout << "Default arg: " << DEFAULT_VERBOSITY_LEVEL_CHOICE;
    cout << "\n";
  }
}

/* namespaced functions: argument parsing *************************************/

void util::printArgv(int argc, char *argv[]) {
  cout << "c argv:";
  for (Int i = 0; i < argc; i++) {
    cout << " " << argv[i];
  }
  cout << "\n";
}

string util::getWeightFormatName(WeightFormat weightFormat) {
  switch (weightFormat) {
    case WeightFormat::UNWEIGHTED: {
      return "UNWEIGHTED";
    }
    case WeightFormat::MINIC2D: {
      return "MINIC2D";
    }
    case WeightFormat::CACHET: {
      return "CACHET";
    }
    case WeightFormat::WCNF: {
      return "WCNF";
    }
    case WeightFormat::WPCNF: {
      return "WPCNF";
    }
    default: {
      showError("illegal weightFormat");
      return "";
    }
  }
}

string util::getPlanningStrategyName(PlanningStrategy planningStrategy) {
  switch (planningStrategy) {
    case PlanningStrategy::FIRST_JOIN_TREE: {
      return "FIRST_JOIN_TREE";
    }
    case PlanningStrategy::TIMING: {
      return "TIMING";
    }
    default: {
      showError("illegal planningStrategy");
      return "";
    }
  }
}

string util::getClusteringHeuristicName(ClusteringHeuristic clusteringHeuristic) {
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::BUCKET_LIST: {
      return "BUCKET_LIST";
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      return "BUCKET_TREE";
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      return "BOUQUET_LIST";
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      return "BOUQUET_TREE";
    }
    default: {
      showError("illegal clusteringHeuristic");
      return "";
    }
  }
}

string util::getVarOrderingHeuristicName(VarOrderingHeuristic varOrderingHeuristic) {
  switch (varOrderingHeuristic) {
    case VarOrderingHeuristic::APPEARANCE: {
      return "APPEARANCE";
    }
    case VarOrderingHeuristic::DECLARATION: {
      return "DECLARATION";
    }
    case VarOrderingHeuristic::RANDOM: {
      return "RANDOM";
    }
    case VarOrderingHeuristic::MCS: {
      return "MCS";
    }
    case VarOrderingHeuristic::LEXP: {
      return "LEXP";
    }
    case VarOrderingHeuristic::LEXM: {
      return "LEXM";
    }
    case VarOrderingHeuristic::MINFILL: {
      return "MINFILL";
    }
    default: {
      showError("util::getVarOrderingHeuristicName");
      return "";
    }
  }
}

string util::getDdPackageName(DdPackage ddPackage) {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return "CUDD";
    }
    case DdPackage::SYLVAN: {
      return "SYLVAN";
    }
    default: {
      showError("illegal ddPackage");
      return "";
    }
  }
}

string util::getJoinPriorityName(JoinPriority joinPriority) {
  switch (joinPriority) {
    case JoinPriority::ARBITRARY: {
      return "ARBITRARY";
    }
    case JoinPriority::SMALLEST_FIRST: {
      return "SMALLEST_FIRST";
    }
    case JoinPriority::LARGEST_FIRST: {
      return "LARGEST_FIRST";
    }
    default: {
      util::showError("illegal joinPriority");
      return "";
    }
  }
}

string util::getVerbosityLevelName(Int verbosityLevel) {
  switch (verbosityLevel) {
    case 0: {
      return "solution only";
    }
    case 1: {
      return "parsed info as well";
    }
    case 2: {
      return "cnf clauses and literal weights as well";
    }
    case 3: {
      return "input lines as well";
    }
    default: {
      showError("illegal verbosityLevel");
      return "";
    }
  }
}

/* namespaced functions: cnf **************************************************/

Int util::getCnfVar(Int literal) {
  if (literal == 0) showError("literal == 0 | util::getCnfVar");
  return abs(literal);
}

Set<Int> util::getClauseCnfVars(const vector<Int> &clause) {
  Set<Int> cnfVars;
  for (Int literal : clause) cnfVars.insert(getCnfVar(literal));
  return cnfVars;
}

Set<Int> util::getClusterCnfVars(const vector<Int> &cluster, const vector<vector<Int>> &clauses) {
  Set<Int> cnfVars;
  for (Int clauseIndex : cluster) unionize(cnfVars, getClauseCnfVars(clauses.at(clauseIndex)));
  return cnfVars;
}

bool util::appearsIn(Int cnfVar, const vector<Int> &clause) {
  for (Int literal : clause) if (getCnfVar(literal) == cnfVar) return true;
  return false;
}

bool util::isPositiveLiteral(Int literal) {
  if (literal == 0) showError("literal == 0 | util::isPositiveLiteral");
  return literal > 0;
}

Int util::getLiteralRank(Int literal, const vector<Int> &cnfVarOrdering) {
  Int cnfVar = getCnfVar(literal);
  auto it = std::find(cnfVarOrdering.begin(), cnfVarOrdering.end(), cnfVar);
  if (it == cnfVarOrdering.end()) showError("cnfVar not found in cnfVarOrdering");
  return it - cnfVarOrdering.begin();
}

Int util::getMinClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering) {
  Int minRank = DUMMY_MAX_INT;
  for (Int literal : clause) {
    Int rank = getLiteralRank(literal, cnfVarOrdering);
    minRank = std::min(minRank, rank);
  }
  return minRank;
}

Int util::getMaxClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering) {
  Int maxRank = DUMMY_MIN_INT;
  for (Int literal : clause) {
    Int rank = getLiteralRank(literal, cnfVarOrdering);
    maxRank = std::max(maxRank, rank);
  }
  return maxRank;
}

void util::printClause(const vector<Int> &clause) {
  for (Int literal : clause) {
    cout << std::right << std::setw(5) << literal << " ";
  }
  cout << "\n";
}

void util::printCnf(const vector<vector<Int>> &clauses) {
  printComment("cnf {");
  for (Int i = 0; i < clauses.size(); i++) {
    cout << "c\t" "clause ";
    cout << std::right << std::setw(5) << i + 1 << " : ";
    printClause(clauses.at(i));
  }
  printComment("}");
}

void util::printLiteralWeights(const Map<Int, Float> &literalWeights) {
  Int maxCnfVar = DUMMY_MIN_INT;
  for (const std::pair<Int, Float> &kv : literalWeights) {
    Int cnfVar = kv.first;
    maxCnfVar = std::max(maxCnfVar, cnfVar);
  }

  printComment("literalWeights {");
  cout << std::right;
  for (Int cnfVar = 1; cnfVar <= maxCnfVar; cnfVar++) {
    cout << "c " << std::right << std::setw(10) << cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(cnfVar) << "\n";
    cout << "c " << std::right << std::setw(10) << -cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(-cnfVar) << "\n";
  }
  printComment("}");
}

/* namespaced functions: timing ***********************************************/

TimePoint util::getTimePoint() {
  return std::chrono::steady_clock::now();
}

Float util::getSeconds(TimePoint startTime) {
  TimePoint endTime = getTimePoint();
  return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
}

void util::printDuration(TimePoint startTime) {
  printThickLine();
  printRow("seconds", getSeconds(startTime));
  printThickLine();
}

void util::handleSignal(int signal) {
  cout << "\n";
  util::printDuration(startTime);
  cout << "\n";
  showError("received OS signal " + to_string(signal) + "; terminated");
}

/* namespaced functions: error handling ***************************************/

void util::showWarning(const string &message, bool commented) {
  printBoldLine(commented);
  printComment("MY_WARNING: " + message, 0, 1, commented);
  printBoldLine(commented);
}

void util::showError(const string &message, bool commented) {
  throw MyError(message, commented);
}

/* namespace parallelizing *****************************************************/

void parallelizing::initializeSylvan(Int workerCount) {
  lace_init(workerCount, 0);
  lace_startup(0, NULL, NULL);
  sylvan::sylvan_set_limits(512*1024*1024, 1, 5); // use at most 512 MB, nodes:cache ratio 2:1, initial size 1/32 of maximum
  sylvan::sylvan_init_package();
  sylvan::sylvan_init_mtbdd();
}

void parallelizing::testSylvan() {
  Dd x2 = Dd::getVarDd(2);
  Dd x1 = Dd::getVarDd(1);

  Dd d = x2.getDisjunction(x1);
  d.writeDotFile();
}

void parallelizing::quitSylvan() {
  sylvan::sylvan_stats_report(stdout);
  sylvan::sylvan_quit();
  lace_exit();
}

/* classes ********************************************************************/

/* class Dd ******************************************************************/

Cudd Dd::mgr;

void Dd::testPriorityQueue(std::priority_queue<Dd, vector<Dd>> &dds) {
  cout << "c testing join priortiy queue: (top) ";
  while (!dds.empty()) {
    cout << dds.top().countNodes() << " ";
    dds.pop();
  }
  cout << "(bottom)\n";
}

Dd Dd::getConstantDd(Float c) {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(mgr.constant(c));
    }
    case DdPackage::SYLVAN: {
      return Dd(Mtbdd::doubleTerminal(c));
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getDummyDd() {
  return getConstantDd(DUMMY_MIN_INT);
}

Dd Dd::getZeroDd() {
  return getConstantDd(0);
}

Dd Dd::getOneDd() {
  return getConstantDd(1);
}

Dd Dd::getVarDd(Int ddVar) {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(mgr.addVar(ddVar));
    }
    case DdPackage::SYLVAN: {
      Mtbdd dd = mtbdd_makenode(ddVar, getZeroDd().mtbdd.GetMTBDD(), getOneDd().mtbdd.GetMTBDD()); // (ddVar, low, high)
      return Dd(dd);
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getNegativeLiteralDd(Int ddVar) {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(~getVarDd(ddVar).cuadd);
    }
    case DdPackage::SYLVAN: {
      Mtbdd dd = mtbdd_makenode(ddVar, getOneDd().mtbdd.GetMTBDD(), getZeroDd().mtbdd.GetMTBDD()); // (ddVar, low, high)
      return Dd(dd);
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Cudd Dd::getMgr() {
  return mgr;
}

ADD Dd::getCuadd() const {
  return cuadd;
}

Mtbdd Dd::getMtbdd() const {
  return mtbdd;
}

Dd::Dd(const ADD &cuadd) {
  this->cuadd = cuadd;
}

Dd::Dd(const Mtbdd &mtbdd) {
  this->mtbdd = mtbdd;
}

Dd::Dd(const Dd &add) {
  this->cuadd = add.cuadd;
  this->mtbdd = add.mtbdd;
}

Int Dd::countNodes() const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return cuadd.nodeCount();
    }
    case DdPackage::SYLVAN: {
      return mtbdd.NodeCount();
    }
    default: {
      util::showError("illegal ddPackage");
      return DUMMY_MIN_INT;
    }
  }
}

bool Dd::operator<(const Dd &right) const {
  switch (joinPriority) {
    case JoinPriority::SMALLEST_FIRST: { // top = rightmost = smallest
      return countNodes() > right.countNodes();
    }
    case JoinPriority::LARGEST_FIRST: { // top = rightmost = largest
      return countNodes() < right.countNodes();
    }
    default: {
      util::showError("unimplemented joinPriority: " + util::getJoinPriorityName(joinPriority));
      return false;
    }
  }
}

Float Dd::getTerminalValue() const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return (cuadd.getNode()->type).value;
    }
    case DdPackage::SYLVAN: {
      return mtbdd_getdouble(mtbdd.GetMTBDD());
    }
    default: {
      util::showError("illegal ddPackage");
      return DUMMY_MIN_INT;
    }
  }
}

Float Dd::countConstDdFloat() const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      ADD minTerminal = cuadd.FindMin();
      ADD maxTerminal = cuadd.FindMax();

      Float minValue = Dd(minTerminal).getTerminalValue();
      Float maxValue = Dd(maxTerminal).getTerminalValue();

      if (minValue != maxValue) {
        util::showError("ADD is nonconst: min value " + std::to_string(minValue) + ", max value " + std::to_string(maxValue));
      }

      return minValue;
    }
    case DdPackage::SYLVAN: {
      if (!mtbdd.isLeaf()) {
        util::showError("mtbdd is not leaf");
      }

      return getTerminalValue();
    }
    default: {
      util::showError("illegal ddPackage");
      return DUMMY_MIN_INT;
    }
  }
}

Dd Dd::getDisjunction(const Dd &add) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(cuadd | add.cuadd);
    }
    case DdPackage::SYLVAN: {
      return Dd(mtbdd.Max(add.mtbdd));
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getConjunction(const Dd &add) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(cuadd & add.cuadd);
    }
    case DdPackage::SYLVAN: {
      return Dd(mtbdd.Min(add.mtbdd));
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getComposition(Int ddVar, bool isTrue) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      ADD constraint = isTrue ? mgr.addOne() : mgr.addZero();
      return Dd(cuadd.Compose(constraint, ddVar));
    }
    case DdPackage::SYLVAN: {
      MtbddMap m;
      m.put(ddVar, isTrue ? Mtbdd::mtbddOne() : Mtbdd::mtbddZero());
      return Dd(mtbdd.Compose(m));
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getProduct(const Dd &add) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(cuadd * add.cuadd);
    }
    case DdPackage::SYLVAN: {
      return Dd(mtbdd * add.mtbdd);
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getMax(const Dd &add) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(cuadd.Maximum(add.cuadd));
    }
    case DdPackage::SYLVAN: {
      return Dd(mtbdd.Max(add.mtbdd));
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Dd Dd::getSum(const Dd &add) const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return Dd(cuadd + add.cuadd);
    }
    case DdPackage::SYLVAN: {
      return Dd(mtbdd + add.mtbdd);
    }
    default: {
      util::showError("illegal ddPackage");
      return getDummyDd();
    }
  }
}

Set<Int> Dd::getSupport() const {
  switch (ddPackage) {
    case DdPackage::CUDD: {
      return util::getSupport(cuadd);
    }
    case DdPackage::SYLVAN: {
      Set<Int> supportSet;
      Mtbdd cube = mtbdd.Support(); // the cube of all variables that appear in the MTBDD
      while (!cube.isOne()) {
        supportSet.insert(cube.TopVar());
        cube = cube.Then();
      }
      return supportSet;
    }
    default: {
      util::showError("illegal ddPackage");
      return Set<Int>();
    }
  }
}

Dd Dd::getAbstraction(Int ddVar, const vector<Int> &ddVarToCnfVarMap, const Map<Int, Float> &literalWeights, bool additive) const {
  Int cnfVar = ddVarToCnfVarMap.at(ddVar);
  Dd positiveWeight = getConstantDd(literalWeights.at(cnfVar));
  Dd negativeWeight = getConstantDd(literalWeights.at(-cnfVar));

  Dd term1 = positiveWeight.getProduct(getComposition(ddVar, true));
  Dd term2 = negativeWeight.getProduct(getComposition(ddVar, false));

  return additive ? term1.getSum(term2) : term1.getMax(term2);
}

void Dd::writeDotFile(const std::string &dotFileDir) const {
  const string &filePath = dotFileDir + "dd" + std::to_string(dotFileIndex++) + ".dot";
  const char *cFilePath = filePath.c_str();
  FILE *file = fopen(cFilePath, "wb"); // writes to binary file

  switch (ddPackage) {
    case DdPackage::CUDD: { // davidkebo.com/cudd#cudd6
      DdManager *ddManager = mgr.getManager();
      DdNode *ddNode = cuadd.getNode();
      DdNode **ddNodeArray = (DdNode**)malloc(sizeof(DdNode*));
      ddNodeArray[0] = ddNode;
      Cudd_DumpDot(ddManager, 1, ddNodeArray, NULL, NULL, file);
      free(ddNodeArray);
      break;
    }
    case DdPackage::SYLVAN: {
      MTBDD cMtbdd = mtbdd.GetMTBDD();
      mtbdd_fprintdot_nc(file, cMtbdd);
      break;
    }
    default: {
      util::showError("illegal ddPackage");
    }
  }

  fclose(file);
  std::cout << "Overwrote file " << filePath << "\n";
}

/* class MyError **************************************************************/

MyError::MyError(const string &message, bool commented) {
  util::printBoldLine(commented);
  util::printComment("MY_ERROR: " + message, 0, 1, commented);
  util::printBoldLine(commented);
}
