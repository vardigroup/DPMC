/* inclusions *****************************************************************/

#include "../interface/main_addmc.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

void OptionDict::printOptionalOptions() const {
  cout << " Optional options:\n";
  util::printHelpOption();
  util::printCnfFileOption();
  util::printWeightFormatOption();
  util::printOutputWeightFormatOption();
  util::printJtFileOption();
  util::printJtWaitOption();
  util::printPerformanceFactorOption();
  util::printOutputFormatOption();
  util::printClusteringHeuristicOption();
  util::printCnfVarOrderingHeuristicOption();
  util::printDdVarOrderingHeuristicOption();
  util::printRandomSeedOption();
  util::printVerbosityLevelOption();
}

void OptionDict::printHelp() const {
  cout << options->help({
    REQUIRED_OPTION_GROUP,
    // OPTIONAL_OPTION_GROUP
  });
  printOptionalOptions();
}

void OptionDict::printWelcome() const {
  bool commented = !helpFlag;

  printThickLine(commented);

  printComment("ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')", 0, 1, commented);

  const string &version = "v1.1.0"; // https://github.com/vardigroup/ADDMC/releases/tag/v1.0.0
  const string &date = "2020/07/20";
  printComment("Version " + version + ", released on " + date, 0, 1, commented);

  printThickLine(commented);
}

OptionDict::OptionDict(int argc, char *argv[]) {
  options = new cxxopts::Options("addmc", "");

  options->add_options(OPTIONAL_OPTION_GROUP)
    (HELP_OPTION, "help")
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
    (CNF_FILE_OPTION, "", cxxopts::value<string>()->default_value(STDIN_CONVENTION))
    (WEIGHT_FORMAT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (OUTPUT_WEIGHT_FORMAT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (JT_FILE_OPTION, "", cxxopts::value<string>()->default_value(DUMMY_STR))
    (JT_WAIT_DURAION_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_JT_WAIT_SECONDS)))
    (PERFORMANCE_FACTOR_OPTION, "performance factor", cxxopts::value<string>()->default_value(to_string(DEFAULT_PERFORMANCE_FACTOR)))
    (OUTPUT_FORMAT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_OUTPUT_FORMAT_CHOICE)))
    (CLUSTERING_HEURISTIC_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_CLUSTERING_HEURISTIC_CHOICE)))
    (CLUSTER_VAR_ORDER_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE)))
    (DIAGRAM_VAR_ORDER_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)))
    (RANDOM_SEED_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
  ;

  cxxopts::ParseResult result = options->parse(argc, argv);

  helpFlag = result["h"].as<bool>();
  verbosityLevelOption = std::stoll(result[VERBOSITY_LEVEL_OPTION].as<string>());

  printWelcome();

  cnfFilePath = result[CNF_FILE_OPTION].as<string>();
  jtFilePath = result[JT_FILE_OPTION].as<string>();
  if (cnfFilePath == jtFilePath) {
    showError("options --" + CNF_FILE_OPTION + " and --" + JT_FILE_OPTION + " must have distinct args", !helpFlag);
  }

  weightFormatOption = std::stoll(result[WEIGHT_FORMAT_OPTION].as<string>());
  outputWeightFormatOption = std::stoll(result[OUTPUT_WEIGHT_FORMAT_OPTION].as<string>());
  jtWaitSeconds = std::stold(result[JT_WAIT_DURAION_OPTION].as<string>());
  performanceFactor = std::stold(result[PERFORMANCE_FACTOR_OPTION].as<string>());
  outputFormatOption = std::stoll(result[OUTPUT_FORMAT_OPTION].as<string>());
  clusteringHeuristicOption = std::stoll(result[CLUSTERING_HEURISTIC_OPTION].as<string>());
  cnfVarOrderingHeuristicOption = std::stoll(result[CLUSTER_VAR_ORDER_OPTION].as<string>());
  ddVarOrderingHeuristicOption = std::stoll(result[DIAGRAM_VAR_ORDER_OPTION].as<string>());
  randomSeedOption = std::stoll(result[RANDOM_SEED_OPTION].as<string>());
}

/* namespaces *****************************************************************/

/* namespace testing **********************************************************/

void testing::test() {
  const Cnf cnf("../examples/F-CACHET.cnf", WeightFormat::CACHET);

  VarOrderingHeuristic ddVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_CHOICES.at(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE);
  VarOrderingHeuristic cnfVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_CHOICES.at(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE);

  MonolithicCounter monolithicCounter(ddVarOrderingHeuristic, false);
  LinearCounter linearCounter(ddVarOrderingHeuristic, false);
  BucketCounter bucketListCounter(false, cnfVarOrderingHeuristic, false, ddVarOrderingHeuristic, false);
  BucketCounter bucketTreeCounter(true, cnfVarOrderingHeuristic, false, ddVarOrderingHeuristic, false);
  BouquetCounter bouquetListCounter(false, cnfVarOrderingHeuristic, false, ddVarOrderingHeuristic, false);
  BouquetCounter bouquetTreeCounter(true, cnfVarOrderingHeuristic, false, ddVarOrderingHeuristic, false);

  Float m = monolithicCounter.getModelCount(cnf);
  Float l = linearCounter.getModelCount(cnf);
  Float bel = bucketListCounter.getModelCount(cnf);
  Float bet = bucketTreeCounter.getModelCount(cnf);
  Float bml = bouquetListCounter.getModelCount(cnf);
  Float bmt = bouquetTreeCounter.getModelCount(cnf);

  vector<Float> counts = {m, l, bel, bet, bml, bmt};
  for (Int i = 0; i < counts.size(); i++) {
    for (Int j = i + 1; j < counts.size(); j++) {
      if (counts.at(i) != counts.at(j)) {
        showWarning("different model counts");
      }
    }
  }

  cout << "\n";
  cout << std::left << std::setw(30) << "monolithic model count" << m << "\n";
  cout << std::left << std::setw(30) << "linear model count" << l << "\n";
  cout << std::left << std::setw(30) << "bucket list model count" << bel << "\n";
  cout << std::left << std::setw(30) << "bucket tree model count" << bet << "\n";
  cout << std::left << std::setw(30) << "bouquet list model count" << bml << "\n";
  cout << std::left << std::setw(30) << "bouquet tree model count" << bmt << "\n";
}

/* namespace solving **********************************************************/

void solving::solveOptions(
  const string &cnfFilePath,
  Int weightFormatOption,
  Int outputWeightFormatOption,
  const string &jtFilePath,
  Float jtWaitSeconds,
  Float performanceFactor,
  Int outputFormatOption,
  Int clusteringHeuristicOption,
  Int cnfVarOrderingHeuristicOption,
  Int ddVarOrderingHeuristicOption
) {
  WeightFormat weightFormat;
  try {
    weightFormat = WEIGHT_FORMAT_CHOICES.at(weightFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal weightFormatOption: " + to_string(weightFormatOption));
  }

  WeightFormat outputWeightFormat;
  try {
    outputWeightFormat = WEIGHT_FORMAT_CHOICES.at(outputWeightFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal outputWeightFormatOption: " + to_string(outputWeightFormatOption));
  }

  OutputFormat outputFormat;
  try {
    outputFormat = OUTPUT_FORMAT_CHOICES.at(outputFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal outputFormatOption: " + to_string(outputFormatOption));
  }

  ClusteringHeuristic clusteringHeuristic;
  try {
    clusteringHeuristic = CLUSTERING_HEURISTIC_CHOICES.at(clusteringHeuristicOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal clusteringHeuristicOption: " + to_string(clusteringHeuristicOption));
  }

  VarOrderingHeuristic cnfVarOrderingHeuristic;
  bool inverseCnfVarOrdering;
  try {
    cnfVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_CHOICES.at(abs(cnfVarOrderingHeuristicOption));
    inverseCnfVarOrdering = cnfVarOrderingHeuristicOption < 0;
  }
  catch (const std::out_of_range &) {
    showError("illegal cnfVarOrderingHeuristicOption: " + to_string(cnfVarOrderingHeuristicOption));
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

  if (verbosityLevel >= 1) {
    printComment("Reading command-line options...", 1);

    /* required: */
    util::printRow("cnfFilePath", cnfFilePath);

    /* optional: */
    util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
    util::printRow("outputWeightFormat", util::getWeightFormatName(outputWeightFormat));
    util::printRow("jtFilePath", jtFilePath);
    util::printRow("jtWaitSeconds", jtWaitSeconds);
    util::printRow("performanceFactor", performanceFactor);
    util::printRow("outputFormat", util::getOutputFormatName(outputFormat));
    util::printRow("clustering", util::getClusteringHeuristicName(clusteringHeuristic));
    util::printRow("clusterVarOrder", util::getVarOrderingHeuristicName(cnfVarOrderingHeuristic));
    util::printRow("inverseClusterVarOrder", inverseCnfVarOrdering);
    util::printRow("diagramVarOrder", util::getVarOrderingHeuristicName(ddVarOrderingHeuristic));
    util::printRow("inverseDiagramVarOrder", inverseDdVarOrdering);
    util::printRow("randomSeed", randomSeed);
  }

  const Cnf cnf(cnfFilePath, weightFormat);

  if (outputFormat == OutputFormat::WEIGHTED_FORMULA) {
    cnf.printWeightedFormula(outputWeightFormat);
    return;
  }

  if (outputFormat == OutputFormat::MODEL_COUNT && jtFilePath != DUMMY_STR) {
    const JoinTreeReader joinTreeReader(jtFilePath, jtWaitSeconds, performanceFactor, cnf.getClauses());
    JoinTreeCounter joinTreeCounter(
      joinTreeReader.getJoinTreeRoot(),
      ddVarOrderingHeuristic,
      inverseDdVarOrdering
    );
    joinTreeCounter.output(cnf, outputFormat);
    return;
  }

  switch (clusteringHeuristic) {
    case ClusteringHeuristic::MONOLITHIC: {
      MonolithicCounter monolithicCounter(ddVarOrderingHeuristic, inverseDdVarOrdering);
      monolithicCounter.output(cnf, outputFormat);
      break;
    }
    case ClusteringHeuristic::LINEAR: {
      LinearCounter linearCounter(ddVarOrderingHeuristic, inverseDdVarOrdering);
        linearCounter.output(cnf, outputFormat);
      break;
    }
    case ClusteringHeuristic::BUCKET_LIST: {
      BucketCounter bucketCounter(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bucketCounter.output(cnf, outputFormat);
      break;
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      BucketCounter bucketCounter(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bucketCounter.output(cnf, outputFormat);
      break;
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      BouquetCounter bouquetCounter(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bouquetCounter.output(cnf, outputFormat);
      break;
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      BouquetCounter bouquetCounter(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bouquetCounter.output(cnf, outputFormat);
      break;
    }
    default: {
      showError("illegal clusteringHeuristic");
    }
  }
}

void solving::solveCommand(int argc, char *argv[]) {
  OptionDict optionDict(argc, argv);

  randomSeed = optionDict.randomSeedOption; // global variable
  verbosityLevel = optionDict.verbosityLevelOption; // global variable
  startTime = util::getTimePoint(); // global variable

  if (optionDict.helpFlag) {
    optionDict.printHelp();
  }
  else {
    printComment("Process ID of this main program:", 1);
    printComment("pid " + to_string(getpid()));

    solveOptions(
      optionDict.cnfFilePath,
      optionDict.weightFormatOption,
      optionDict.outputWeightFormatOption,
      optionDict.jtFilePath,
      optionDict.jtWaitSeconds,
      optionDict.performanceFactor,
      optionDict.outputFormatOption,
      optionDict.clusteringHeuristicOption,
      optionDict.cnfVarOrderingHeuristicOption,
      optionDict.ddVarOrderingHeuristicOption
    );
    cout << "\n";

    util::printDuration(startTime);
  }
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]) {
  cout << std::unitbuf; // enables automatic flushing

  // mainVisual(argc, argv);
  // testing::test();
  solving::solveCommand(argc, argv);
}
