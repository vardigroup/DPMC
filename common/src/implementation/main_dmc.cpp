/* inclusions *****************************************************************/

#include "main_dmc.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

void OptionDict::printOptionalOptions() const {
  cout << " Optional options:\n";
  util::printWeightFormatOption();
  util::printJtWaitOption();
  util::printPerformanceFactorOption();
  util::printDdVarOrderingHeuristicOption();
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
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
    (WEIGHT_FORMAT_OPTION, "weight format", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (JT_WAIT_DURAION_OPTION, "jt wait duration in seconds", cxxopts::value<Float>()->default_value(to_string(DEFAULT_JT_WAIT_SECONDS)))
    (PERFORMANCE_FACTOR_OPTION, "performance factor", cxxopts::value<Float>()->default_value(to_string(DEFAULT_PERFORMANCE_FACTOR)))
    (DIAGRAM_VAR_ORDER_OPTION, "diagram variable order heuristic", cxxopts::value<string>()->default_value(to_string(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)))
    (RANDOM_SEED_OPTION, "random seed", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
  ;

  cxxopts::ParseResult result = options->parse(argc, argv);

  verbosityLevelOption = std::stoll(result[VERBOSITY_LEVEL_OPTION].as<string>());

  hasEnoughArgs = result.count(CNF_FILE_OPTION) && result.count(JT_FILE_OPTION)
    // && result.count(PERFORMANCE_FACTOR_OPTION)
  ;
  if (hasEnoughArgs) {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();
    jtFilePath = result[JT_FILE_OPTION].as<string>();
    weightFormatOption = std::stoll(result[WEIGHT_FORMAT_OPTION].as<string>());
    jtWaitSeconds = (result[JT_WAIT_DURAION_OPTION].as<Float>());
    performanceFactor = result[PERFORMANCE_FACTOR_OPTION].as<Float>();
    if (performanceFactor <= 0) { // cxxopts underflow
      performanceFactor = DEFAULT_PERFORMANCE_FACTOR;
    }
    ddVarOrderingHeuristicOption = std::stoll(result[DIAGRAM_VAR_ORDER_OPTION].as<string>());
    randomSeedOption = std::stoll(result[RANDOM_SEED_OPTION].as<string>());
  }
}

/* namespaces *****************************************************************/

/* namespace solving **********************************************************/

void solving::solveOptions(
  const string &cnfFilePath,
  const string &jtFilePath,
  Int weightFormatOption,
  Float jtWaitSeconds,
  Float performanceFactor,
  Int ddVarOrderingHeuristicOption
) {
  WeightFormat weightFormat;
  try {
    weightFormat = WEIGHT_FORMAT_CHOICES.at(weightFormatOption);
  }
  catch (const std::out_of_range &) {
    showError("illegal weightFormatOption: " + to_string(weightFormatOption));
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
    util::printRow("jtFilePath", jtFilePath);

    /* optional: */
    util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
    util::printRow("jtWaitSeconds", jtWaitSeconds);
    util::printRow("performanceFactor", performanceFactor);
    util::printRow("diagramVarOrder", util::getVarOrderingHeuristicName(ddVarOrderingHeuristic));
    util::printRow("inverseDiagramVarOrder", inverseDdVarOrdering);
  }

  const Cnf cnf(cnfFilePath, weightFormat);

  const JoinTreeReader joinTreeReader(jtFilePath, jtWaitSeconds, performanceFactor, cnf.getClauses());
  JoinTreeCounter joinTreeCounter(
    joinTreeReader.getJoinTreeRoot(),
    ddVarOrderingHeuristic,
    inverseDdVarOrdering
  );

  joinTreeCounter.output(cnf, OutputFormat::MODEL_COUNT);
}

void solving::solveCommand(int argc, char *argv[]) {
  OptionDict optionDict(argc, argv);
  randomSeed = optionDict.randomSeedOption; // global variable
  verbosityLevel = optionDict.verbosityLevelOption; // global variable
  if (optionDict.hasEnoughArgs) {
    printComment("Process ID of this main program:", 1);
    printComment("pid " + to_string(getpid()));

    startTime = util::getTimePoint(); // global variable
    solveOptions(
      optionDict.cnfFilePath,
      optionDict.jtFilePath,
      optionDict.weightFormatOption,
      optionDict.jtWaitSeconds,
      optionDict.performanceFactor,
      optionDict.ddVarOrderingHeuristicOption
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
  printComment("DMC: Diagram Model Counter");

  const string &version = "v1.0.0";
  const string &date = "2020/07/20";
  printComment("Version " + version + ", released on " + date);

  printThickLine();

  solving::solveCommand(argc, argv);
}
