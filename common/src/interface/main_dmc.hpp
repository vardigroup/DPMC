/* inclusions *****************************************************************/

#include "../lib/cxxopts.hpp"

#include "counter.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  bool hasEnoughArgs;

  /* required: */
  string cnfFilePath;
  string jtFilePath;

  /* optional: */
  Int weightFormatOption;
  Float jtWaitSeconds;
  Float performanceFactor;
  Int ddVarOrderingHeuristicOption;
  Int randomSeedOption;
  Int verbosityLevelOption;

  cxxopts::Options *options;
  Int cnfFilePathCount;
  Int jtFilePathCount;

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
    Float jtWaitSeconds,
    Float performanceFactor,
    Int ddVarOrderingHeuristicOption
  );
  void solveCommand(int argc, char *argv[]);
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]);
