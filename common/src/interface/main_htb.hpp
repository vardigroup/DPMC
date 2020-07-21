/* inclusions *****************************************************************/

#include "../../lib/cxxopts.hpp"

#include "counter.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  /* required: */
  string cnfFilePath;

  /* optional: */
  Int weightFormatOption;
  Int clusteringHeuristicOption;
  Int cnfVarOrderingHeuristicOption;
  Int randomSeedOption;
  Int verbosityLevelOption;

  cxxopts::Options *options;
  Int cnfFilePathCount;

  bool hasEnoughArgs() const;
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
