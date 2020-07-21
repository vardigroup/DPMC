/* inclusions *****************************************************************/

#include "../../lib/cxxopts.hpp"

#include "counter.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  /* optional: */
  bool helpFlag;
  string cnfFilePath;
  Int weightFormatOption;
  Int outputWeightFormatOption;
  string jtFilePath;
  Float jtWaitSeconds;
  Float performanceFactor;
  Int outputFormatOption;
  Int clusteringHeuristicOption;
  Int cnfVarOrderingHeuristicOption;
  Int ddVarOrderingHeuristicOption;
  Int randomSeedOption;
  Int verbosityLevelOption;

  cxxopts::Options *options;

  void printOptionalOptions() const;
  void printHelp() const;
  void printWelcome() const;
  OptionDict(int argc, char *argv[]);
};

/* namespaces *****************************************************************/

namespace testing {
  void test();
}

namespace solving {
  void solveOptions(
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
  );
  void solveCommand(int argc, char *argv[]);
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]);
