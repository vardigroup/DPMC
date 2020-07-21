/* inclusions *****************************************************************/

#include "../interface/main_htb.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

bool OptionDict::hasEnoughArgs() const {
  return cnfFilePathCount > 0;
}

void OptionDict::printOptionalOptions() const {
  cout << " Optional options:\n";
  util::printWeightFormatOption();
  util::printClusteringHeuristicOption();
  util::printCnfVarOrderingHeuristicOption();
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
  options = new cxxopts::Options("htb", "");

  options->add_options(REQUIRED_OPTION_GROUP)
    (CNF_FILE_OPTION, "cnf file path (input)", cxxopts::value<string>())
  ;

  options->add_options(OPTIONAL_OPTION_GROUP)
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
    (WEIGHT_FORMAT_OPTION, "weight format", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (CLUSTERING_HEURISTIC_OPTION, "clustering heuristic", cxxopts::value<string>()->default_value(to_string(DEFAULT_CLUSTERING_HEURISTIC_CHOICE)))
    (CLUSTER_VAR_ORDER_OPTION, "cluster variable order heuristic", cxxopts::value<string>()->default_value(to_string(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE)))
    (RANDOM_SEED_OPTION, "random seed", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
  ;

  cxxopts::ParseResult result = options->parse(argc, argv);

  verbosityLevelOption = std::stoll(result[VERBOSITY_LEVEL_OPTION].as<string>());

  cnfFilePathCount = result.count(CNF_FILE_OPTION);
  if (hasEnoughArgs()) {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();
    weightFormatOption = std::stoll(result[WEIGHT_FORMAT_OPTION].as<string>());
    clusteringHeuristicOption = std::stoll(result[CLUSTERING_HEURISTIC_OPTION].as<string>());
    cnfVarOrderingHeuristicOption = std::stoll(result[CLUSTER_VAR_ORDER_OPTION].as<string>());
    randomSeedOption = std::stoll(result[RANDOM_SEED_OPTION].as<string>());
  }
}

/* namespaces *****************************************************************/

/* namespace solving **********************************************************/

void solving::solveOptions(
  const string &cnfFilePath,
  Int weightFormatOption,
  Int clusteringHeuristicOption,
  Int cnfVarOrderingHeuristicOption
) {
  WeightFormat weightFormat;
  try {
    weightFormat = WEIGHT_FORMAT_CHOICES.at(weightFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal weightFormatOption: " + to_string(weightFormatOption));
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

  if (verbosityLevel >= 1) {
    printComment("Reading command-line options...", 1);

    /* required: */
    util::printRow("cnfFilePath", cnfFilePath);

    /* optional: */
    util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
    util::printRow("clustering", util::getClusteringHeuristicName(clusteringHeuristic));
    util::printRow("clusterVarOrder", util::getVarOrderingHeuristicName(cnfVarOrderingHeuristic));
    util::printRow("inverseClusterVarOrder", inverseCnfVarOrdering);
  }

  const Cnf cnf(cnfFilePath, weightFormat);

  VarOrderingHeuristic ddVarOrderingHeuristic = VarOrderingHeuristic::DUMMY_VAR_ORDERING_HEURISTIC;
  bool inverseDdVarOrdering = false; // dummy
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::MONOLITHIC: {
      MonolithicCounter monolithicCounter(ddVarOrderingHeuristic, inverseDdVarOrdering);
      monolithicCounter.output(cnf, OutputFormat::JOIN_TREE);
      break;
    }
    case ClusteringHeuristic::LINEAR: {
      LinearCounter linearCounter(ddVarOrderingHeuristic, inverseDdVarOrdering);
        linearCounter.output(cnf, OutputFormat::JOIN_TREE);
      break;
    }
    case ClusteringHeuristic::BUCKET_LIST: {
      BucketCounter bucketCounter(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bucketCounter.output(cnf, OutputFormat::JOIN_TREE);
      break;
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      BucketCounter bucketCounter(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bucketCounter.output(cnf, OutputFormat::JOIN_TREE);
      break;
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      BouquetCounter bouquetCounter(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bouquetCounter.output(cnf, OutputFormat::JOIN_TREE);
      break;
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      BouquetCounter bouquetCounter(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering, ddVarOrderingHeuristic, inverseDdVarOrdering);
      bouquetCounter.output(cnf, OutputFormat::JOIN_TREE);
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
  if (optionDict.hasEnoughArgs()) {
    printComment("Process ID of this main program:", 1);
    printComment("pid " + to_string(getpid()));

    startTime = util::getTimePoint(); // global variable
    solveOptions(
      optionDict.cnfFilePath,
      optionDict.weightFormatOption,
      optionDict.clusteringHeuristicOption,
      optionDict.cnfVarOrderingHeuristicOption
    );
    cout << "\n";
    util::printDuration(startTime);
  }
  else {
    optionDict.printHelp();
  }
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]) {
  cout << std::unitbuf; // enables automatic flushing

  printThickLine();
  printComment("HTB: Heuristic Tree Builder");

  const string &version = "v1.0.0";
  const string &date = "2020/07/20";
  printComment("Version " + version + ", released on " + date);

  printThickLine();

  solving::solveCommand(argc, argv);
}
