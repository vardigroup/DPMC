/* inclusions *****************************************************************/

#include "../interface/main_dmc.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

void OptionDict::printOptionalOptions() const {
  cout << " Optional options:\n";
  util::printWeightFormatOption();
  util::printPlanningStrategyOption();
  util::printJtWaitOption();
  util::printPerformanceFactorOption();
  util::printDdVarOrderingHeuristicOption();
  util::printDdPackageOption();
  util::printWorkerCountOption();
  util::printJoinPriorityOption();
  util::printRandomSeedOption();
  util::printVerbosityLevelOption();
}

void OptionDict::printHelp() const {
  cout << options->help({
    REQUIRED_OPTION_GROUP,
    // OPTIONAL_OPTION_GROUP
  }) << "\n";
  printOptionalOptions();
}

OptionDict::OptionDict(int argc, char *argv[]) {
  options = new cxxopts::Options("dmc", "");

  options->add_options(REQUIRED_OPTION_GROUP)
    (CNF_FILE_OPTION, "cnf file path (input)", cxxopts::value<string>())
    (JT_FILE_OPTION, "jt file path (to use stdin, type: '--" + JT_FILE_OPTION + " -')", cxxopts::value<string>())
  ;

  options->add_options(OPTIONAL_OPTION_GROUP)
    (WEIGHT_FORMAT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (PLANNING_STRATEGY_OPTION, "", cxxopts::value<string>()->default_value(DEFAULT_PLANNING_STRATEGY_CHOICE))
    (JT_WAIT_DURAION_OPTION, "", cxxopts::value<Float>()->default_value(to_string(DEFAULT_JT_WAIT_SECONDS)))
    (PERFORMANCE_FACTOR_OPTION, "", cxxopts::value<Float>()->default_value(to_string(DEFAULT_PERFORMANCE_FACTOR)))
    (DD_VAR_ORDER_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)))
    (DD_PACKAGE_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_DD_PACKAGE_CHOICE)))
    (WORKER_COUNT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_WORKER_COUNT)))
    (JOIN_PRIORITY_OPTION, "", cxxopts::value<string>()->default_value(DEFAULT_JOIN_PRIORITY_CHOICE))
    (RANDOM_SEED_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
  ;

  cxxopts::ParseResult result = options->parse(argc, argv);


  hasEnoughArgs = result.count(CNF_FILE_OPTION) && result.count(JT_FILE_OPTION);
  if (hasEnoughArgs) {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();
    jtFilePath = result[JT_FILE_OPTION].as<string>();
    weightFormatOption = std::stoll(result[WEIGHT_FORMAT_OPTION].as<string>());
    planningStrategyOption = result[PLANNING_STRATEGY_OPTION].as<string>();
    jtWaitSeconds = (result[JT_WAIT_DURAION_OPTION].as<Float>());
    performanceFactor = result[PERFORMANCE_FACTOR_OPTION].as<Float>();
    if (performanceFactor <= 0) { // possible cxxopts underflow
      performanceFactor = DEFAULT_PERFORMANCE_FACTOR;
    }
    ddVarOrderingHeuristicOption = std::stoll(result[DD_VAR_ORDER_OPTION].as<string>());
    ddPackageOption = std::stoll(result[DD_PACKAGE_OPTION].as<string>());
    workerCount = std::stoll(result[WORKER_COUNT_OPTION].as<string>());
    joinPriorityOption = result[JOIN_PRIORITY_OPTION].as<string>();
    randomSeed = std::stoll(result[RANDOM_SEED_OPTION].as<string>());
    verbosityLevel = std::stoll(result[VERBOSITY_LEVEL_OPTION].as<string>());
  }
}

/* namespaces *****************************************************************/

/* namespace solving **********************************************************/

void solving::solveOptions(
  const string &cnfFilePath,
  const string &jtFilePath,
  Int weightFormatOption,
  const string &planningStrategyOption,
  Float jtWaitSeconds,
  Float performanceFactor,
  Int ddVarOrderingHeuristicOption,
  Int ddPackageOption,
  Int workerCount,
  const string &joinPriorityOption
) {
  WeightFormat weightFormat;
  try {
    weightFormat = WEIGHT_FORMAT_CHOICES.at(weightFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal weightFormatOption: " + to_string(weightFormatOption));
  }

  try {
    planningStrategy = PLANNING_STRATEGY_CHOICES.at(planningStrategyOption); // global variable
  }
  catch (const std::out_of_range &) {
    showError("illegal planningStrategyOption: " + planningStrategyOption);
  }

  VarOrderingHeuristic ddVarOrderingHeuristic;
  bool inverseDdVarOrdering;
  try {
    ddVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_CHOICES.at(abs(ddVarOrderingHeuristicOption));
    inverseDdVarOrdering = ddVarOrderingHeuristicOption < 0;
  }
  catch (const std::out_of_range &) {
    showError("illegal ddVarOrderingHeuristicOption: " + to_string(ddVarOrderingHeuristicOption));
  }

  try {
    ddPackage = DD_PACKAGE_CHOICES.at(ddPackageOption); // global variable
  }
  catch (const std::out_of_range &) {
    showError("illegal ddPackageOption: " + to_string(ddPackageOption));
  }

  if (ddPackage == DdPackage::SYLVAN) parallelizing::initializeSylvan(workerCount);

  try {
    joinPriority = JOIN_PRIORITY_CHOICES.at(joinPriorityOption); // global variable
  }
  catch (const std::out_of_range &) {
    showError("illegal joinPriorityOption: " + joinPriorityOption);
  }

  if (verbosityLevel >= 1) {
    printComment("processing command-line options...", 1);

    /* required: */
    util::printRow("cnfFilePath", cnfFilePath);
    util::printRow("jtFilePath", jtFilePath);

    /* optional: */
    util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
    util::printRow("planningStrategy", util::getPlanningStrategyName(planningStrategy));
    util::printRow("jtWaitSeconds", jtWaitSeconds);
    util::printRow("performanceFactor", performanceFactor);
    util::printRow("diagramVarOrder", util::getVarOrderingHeuristicName(ddVarOrderingHeuristic));
    util::printRow("inverseDiagramVarOrder", inverseDdVarOrdering);
    util::printRow("diagramPackage", util::getDdPackageName(ddPackage));
    util::printRow("workerCount", workerCount);
    util::printRow("joinPriority", util::getJoinPriorityName(joinPriority));
    util::printRow("randomSeed", randomSeed);
  }

  const Cnf cnf(cnfFilePath, weightFormat);

  JoinTreeProcessor *joinTreeProcessor;
  if (planningStrategy == PlanningStrategy::FIRST_JOIN_TREE) {
    joinTreeProcessor = new JoinTreeParser(jtFilePath, cnf.getClauses());
  }
  else {
    joinTreeProcessor = new JoinTreeReader(jtFilePath, jtWaitSeconds, performanceFactor, cnf.getClauses());
  }

  Executor executor(joinTreeProcessor->getJoinTreeRoot(), ddVarOrderingHeuristic, inverseDdVarOrdering);
  executor.outputModelCount(cnf);

  if (ddPackage == DdPackage::SYLVAN) parallelizing::quitSylvan();
}

void solving::solveCommand(int argc, char *argv[]) {
  OptionDict optionDict(argc, argv);
  randomSeed = optionDict.randomSeed; // global variable
  verbosityLevel = optionDict.verbosityLevel; // global variable
  if (optionDict.hasEnoughArgs) {
    util::printArgv(argc, argv);
    startTime = util::getTimePoint(); // global variable
    solveOptions(
      optionDict.cnfFilePath,
      optionDict.jtFilePath,
      optionDict.weightFormatOption,
      optionDict.planningStrategyOption,
      optionDict.jtWaitSeconds,
      optionDict.performanceFactor,
      optionDict.ddVarOrderingHeuristicOption,
      optionDict.ddPackageOption,
      optionDict.workerCount,
      optionDict.joinPriorityOption
    );
    cout << "\n";
    util::printDuration(startTime);
  }
  else {
    cout << "DMC: Diagram Model Counter\n";
    optionDict.printHelp();
  }
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]) {
  cout << std::unitbuf; // enables automatic flushing
  solving::solveCommand(argc, argv);
}
