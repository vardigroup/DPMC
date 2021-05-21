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
  util::printCountOrSampleOption();
  util::printSampleFileOption();
  util::printNumSamplesOption();
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
    (COUNT_OR_SAMPLE_OPTION, "count or sample", cxxopts::value<string>()->default_value(to_string(DEFAULT_COUNT_OR_SAMPLE_CHOICE)))
    (SAMPLE_FILE_OPTION, "sample file", cxxopts::value<string>()->default_value(DEFAULT_SAMPLE_FILE_CHOICE))
    (NUM_SAMPLES_OPTION, "number of samples", cxxopts::value<string>()->default_value(to_string(DEFAULT_NUM_SAMPLES_CHOICE)))
    (VERBOSITY_LEVEL_OPTION, "", cxxopts::value<string>()->default_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE))->implicit_value(to_string(DEFAULT_VERBOSITY_LEVEL_CHOICE + 1)))
    (WEIGHT_FORMAT_OPTION, "weight format", cxxopts::value<string>()->default_value(to_string(DEFAULT_WEIGHT_FORMAT_CHOICE)))
    (JT_WAIT_DURAION_OPTION, "jt wait duration in seconds", cxxopts::value<Float>()->default_value(to_string(DEFAULT_JT_WAIT_SECONDS)))
    (PERFORMANCE_FACTOR_OPTION, "performance factor", cxxopts::value<Float>()->default_value(to_string(DEFAULT_PERFORMANCE_FACTOR)))
    (DIAGRAM_VAR_ORDER_OPTION, "diagram variable order heuristic", cxxopts::value<string>()->default_value(to_string(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)))
    (RANDOM_SEED_OPTION, "random seed", cxxopts::value<string>()->default_value(to_string(DEFAULT_RANDOM_SEED)))
  ;
  printComment("starting parsing options\n");
  cxxopts::ParseResult result = options->parse(argc, argv);
  printComment("finished parsing options\n");
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
    countOrSampleOption = result[COUNT_OR_SAMPLE_OPTION].as<string>()[0];
    sampleFileOption = result[SAMPLE_FILE_OPTION].as<string>();
    numSamplesOption = std::stoll(result[NUM_SAMPLES_OPTION].as<string>());
  }
  printComment("optionsdict created\n");
}

/* namespaces *****************************************************************/

/* namespace solving **********************************************************/

void solving::solveOptions(
  const string &cnfFilePath,
  const string &jtFilePath,
  Int weightFormatOption,
  Float jtWaitSeconds,
  Float performanceFactor,
  Int ddVarOrderingHeuristicOption,
  char countOrSampleOption,
  string sampleFileOption,
  Int numSamplesOption
) {
  printComment("Starting solveOptions..\n");
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
  cout<<"Printing cntsmpl in solveOptions.. "<<countOrSampleOption<<"\n";
  if (countOrSampleOption != 'c' && countOrSampleOption != 's'){
    showError("illegal countOrSampleOption: " + to_string(countOrSampleOption));
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
    util::printRow("countOrSample",countOrSampleOption=='c'?"counting":"sampling");
    util::printRow("sampleFile",sampleFileOption);
    util::printRow("numSamples",numSamplesOption);
  }

  const Cnf cnf(cnfFilePath, weightFormat);
  printComment("cnf formula parssed!\n");
  const JoinTreeReader joinTreeReader(jtFilePath, jtWaitSeconds, performanceFactor, cnf.getClauses());
  JoinTreeCounter joinTreeCounter(
    joinTreeReader.getJoinTreeRoot(),
    ddVarOrderingHeuristic,
    inverseDdVarOrdering
  );
  printComment("starting jointreecounter..\n");
  if (countOrSampleOption == 'c'){
    joinTreeCounter.output(cnf, sampleFileOption, 0, OutputFormat::MODEL_COUNT);
  } else{
    joinTreeCounter.output(cnf, sampleFileOption, numSamplesOption, OutputFormat::SAMPLE);
  }
  
}

void solving::solveCommand(int argc, char *argv[]) {
  printComment("creating optionsdict..\n");
  OptionDict optionDict(argc, argv);
  randomSeed = optionDict.randomSeedOption; // global variable
  verbosityLevel = optionDict.verbosityLevelOption; // global variable
  if (optionDict.hasEnoughArgs) {
    printComment("Process ID of this main program:", 1);
    printComment("pid " + to_string(getpid()));

    startTime = util::getTimePoint(); // global variable
    printComment("Calling solve options\n");
    solveOptions(
      optionDict.cnfFilePath,
      optionDict.jtFilePath,
      optionDict.weightFormatOption,
      optionDict.jtWaitSeconds,
      optionDict.performanceFactor,
      optionDict.ddVarOrderingHeuristicOption,
      optionDict.countOrSampleOption,
      optionDict.sampleFileOption,
      optionDict.numSamplesOption
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
