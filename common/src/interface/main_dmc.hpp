/* inclusions *****************************************************************/

#include "phase.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  bool hasEnoughArgs;

  /* required: */
  string cnfFilePath;
  string jtFilePath;

  /* optional: */
  Int weightFormatOption;
  string planningStrategyOption;
  Float jtWaitSeconds;
  Float performanceFactor;
  Int ddVarOrderingHeuristicOption;
  Int ddPackageOption;
  Int workerCount;
  string joinPriorityOption;
  Int randomSeed;
  Int verbosityLevel;

  cxxopts::Options *options;

  void printOptionalOptions() const;
  void printHelp() const;
  OptionDict(int argc, char *argv[]);
};

/* namespaces *****************************************************************/

namespace solving {
  void solveOptions(
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
  );
  void solveCommand(int argc, char *argv[]);
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]);
