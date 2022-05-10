/* inclusions *****************************************************************/

#include "../interface/main_htb.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

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
    (WEIGHT_FORMAT_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (CLUSTERING_HEURISTIC_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_CLUSTERING_HEURISTIC_CHOICE)))
    (CLUSTER_VAR_ORDER_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE)))
    (RANDOM_SEED_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
  ;

  cxxopts::ParseResult result = options->parse(argc, argv);

  hasEnoughArgs = result.count(CNF_FILE_OPTION) > 0;
  if (hasEnoughArgs) {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();
    weightFormatOption = std::stoll(result[WEIGHT_FORMAT_OPTION].as<string>());
    clusteringHeuristicOption = std::stoll(result[CLUSTERING_HEURISTIC_OPTION].as<string>());
    cnfVarOrderingHeuristicOption = std::stoll(result[CLUSTER_VAR_ORDER_OPTION].as<string>());
    randomSeed = std::stoll(result[RANDOM_SEED_OPTION].as<string>());
    verbosityLevel = std::stoll(result[VERBOSITY_LEVEL_OPTION].as<string>());
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
    printComment("processing command-line options...", 1);

    /* required: */
    util::printRow("cnfFilePath", cnfFilePath);

    /* optional: */
    util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
    util::printRow("clustering", util::getClusteringHeuristicName(clusteringHeuristic));
    util::printRow("clusterVarOrder", util::getVarOrderingHeuristicName(cnfVarOrderingHeuristic));
    util::printRow("inverseClusterVarOrder", inverseCnfVarOrdering);
    util::printRow("randomSeed", randomSeed);
  }

  const Cnf cnf(cnfFilePath, weightFormat);

  switch (clusteringHeuristic) {
    case ClusteringHeuristic::BUCKET_LIST: {
      BucketPlanner bucketPlanner(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering);
      bucketPlanner.outputJoinTree(cnf);
      break;
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      BucketPlanner bucketPlanner(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering);
      bucketPlanner.outputJoinTree(cnf);
      break;
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      BouquetPlanner bouquetPlanner(false, cnfVarOrderingHeuristic, inverseCnfVarOrdering);
      bouquetPlanner.outputJoinTree(cnf);
      break;
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      BouquetPlanner bouquetPlanner(true, cnfVarOrderingHeuristic, inverseCnfVarOrdering);
      bouquetPlanner.outputJoinTree(cnf);
      break;
    }
    default: {
      showError("illegal clusteringHeuristic");
    }
  }
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
      optionDict.weightFormatOption,
      optionDict.clusteringHeuristicOption,
      optionDict.cnfVarOrderingHeuristicOption
    );
    cout << "\n";
    util::printDuration(startTime);
  }
  else {
    cout << "HTB: Heuristic Tree Builder\n";
    optionDict.printHelp();
  }
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]) {
  cout << std::unitbuf; // enables automatic flushing
  solving::solveCommand(argc, argv);
}
