#ifndef ADDSAMPLER_H_
#define ADDSAMPLER_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "sampler/RandomBits.hpp"
#include "sampler/SamplerNodeFactory.hpp"
#include "sampler/utils.hpp"
#include "join.hpp"

using std::vector;
using std::unordered_set;
using std::string;
using std::unordered_map;
using Sampler::SamplerNode;
using Sampler::SamplerNodeFactory;

namespace Sampler{
class Asmt{
	public:
		Asmt(Int nVars_): bits(nVars_,-1){  //-1 is unset default value
		}
		void setNoCheck(bool bit, Int i);
		void set(bool bit, Int i);
		bool get(Int i);
		void clear();
		void printAsmt();
	private:
		vector<int8_t> bits; //not bool becz need to have 3rd 'unset' value (defined to be -1)
};

class ADDSampler{
	public:
		ADDSampler(JoinNode* root_, Cudd mgr_, Int nTotalVars_, Int nApparentVars, unordered_map<Int,Int> c2DVarMap, vector<Int> d2CVarMap, 
			unordered_map<Int, Float> litWeights_, Set<Int> freeVars, bool checkAsmts_, Float startTime);
		
		void buildDataStructures();
		Asmt& drawSample();
		bool checkAsmt();
		void writeAsmtToFile(FILE* ofp);
		//Int getNumVars();
		uint_fast32_t asd;		
	private:
		void createAuxStructures(JoinNode*);
		void createAuxStructure(JoinNode*);
		void createSamplingDAGs(JoinNode*);
		void createSamplingDAGs_rec(SamplerUtils::ADDWrapper&, DdNode*, unordered_set<DdNode*>*);
		void createSamplingDAG(JoinNode*);
		void sampleFromADD(JoinNode*);
		void drawSample_rec(JoinNode*);		
		void reduceADD(SamplerUtils::ADDWrapper& a, ADD& tempADD);
		Float getAsmtVal(SamplerUtils::ADDWrapper& );
		bool checkAsmt_rec(JoinNode*);

		bool checkAsmts;

		Int nTotalVars, nApparentVars;
		vector<SamplerUtils::ADDWrapper> adds;
		Asmt* t;
		DdNode** sVars;
		RandomBits rb;
		SamplerNodeFactory *snFactory;

		JoinNode* jtRoot;
		unordered_map<Int, Int> cnfVarToDdVarMap;
		vector<Int> ddVarToCnfVarMap;
		ADD assignedVarsCube;
		int numAssigned;
		Cudd mgr;
		unordered_map<Int, Float> litWeights;
		Set<Int> freeVars;

		Float startTime, allAuxStructsTime, allDAGsTime;
};
}//end namespace
#endif