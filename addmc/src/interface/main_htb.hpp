/* inclusions *****************************************************************/

#include "phase.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  bool hasEnoughArgs;

  /* required: */
  string cnfFilePath;

  /* optional: */
  Int weightFormatOption;
  Int clusteringHeuristicOption;
  Int cnfVarOrderingHeuristicOption;
  Int randomSeed;
  Int verbosityLevel;

  cxxopts::Options *options;
  Int cnfFilePathCount;

  void printOptionalOptions() const;
  void printHelp() const;
  OptionDict(int argc, char *argv[]);
};

/* namespaces *****************************************************************/

namespace solving {
  void solveOptions(
    const string &cnfFilePath,
    Int weightFormatOption,
    Int clusteringHeuristicOption,
    Int cnfVarOrderingHeuristicOption
  );
  void solveCommand(int argc, char *argv[]);
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]);
