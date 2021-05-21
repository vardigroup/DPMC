#ifndef UTILS_H_
#define UTILS_H_

#include <vector>
#include <unordered_map>
#include <utility>

#include "../../../lib/cudd-3.0.0/cplusplus/cuddObj.hh"
#include "../../../lib/cudd-3.0.0/cudd/cuddInt.h"
#include "../../../lib/cudd-3.0.0/cudd/cudd.h"

#include "sampler/Timers.h"
#include "sampler/SamplerNode.hpp"

using std::unordered_map;
using std::pair;
using Sampler::SamplerNode;

namespace SamplerUtils{
//this ADDWrapper is different from addwrapper in aig2dd.cpp. This one is the same as the one in sample_trace
//but renamed to prevent clash with ADD class in cudd. Also some member variable changes. Eventually use namespacce maybe
//changed from struct so making all public. make members private later.
class ADDWrapper{
	public:

		ADDWrapper(ADD add_);
		ADDWrapper();
		ADD add;	
		unordered_map<DdNode*, SamplerNode*> rootSNs; //vector in case of multiple roots when using reverse ordering. rootSNs[0] is always the sampler node corresponding to the root of the whole ADD.
		unordered_map<DdNode*, vector<std::pair<DdNode*,SamplerNode*>>> allSamplingPreLeaves; //vector (of vector) in case of multiple roots when using reverse ordering
		unordered_map<Int, Int> compressedPerm; // stores mapping from ddvarids to compressed-ddvarids/cmprsdlevels
		Int* compressedInvPerm; // stores mapping from compressedddvarids/cmprsdlvls to cnfvarid,ddvarid pair
		//not explicitly storing mapping from compressed to uncompressed ddvarids. go through cnfvarids for that
		Int startSampleVarLevel, endSampleVarLevel, sampleStructDepth;
		bool precompileSampleDAG;
};

void printPerm(DdManager*);
void printDoubleLine();
string printTimeTaken(string forWhat, Float timeTaken, Int numNewLines);

int Cudd_StringSummary(DdManager * dd, DdNode * f, int n, /**< number of variables for minterm computation */
  string* out /**pointer to string that will contain output*/);
} //end namespace
#endif