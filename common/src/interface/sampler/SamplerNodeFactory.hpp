#ifndef SAMPLERNODEFACTORY_H_
#define SAMPLERNODEFACTORY_H_

#include <cstdint>
#include <unordered_set>
#include <deque>
#include <iostream>
#include "sampler/SamplerNode.hpp"
#include "sampler/GMPNumType.hpp"

using std::unordered_set;
using std::deque;

namespace Sampler{
class SamplerNodeFactory{
	public:
		enum NUMTYPE{
			dbl,long_dbl,gmpz
		};
		static SamplerNodeFactory* getFactory(Int memLimit, SamplerNodeFactory::NUMTYPE SamplerNodeType_);
		void forgetAllUsedNodes();
		void recycleAllUsedNodes();
		SamplerNode* createRoot(Int cmprsdLvl);
		SamplerNode* createRoot(Int cmprsdLvl, vector<Int> missingIndices, unordered_map<Int, Float> litWts);
		SamplerNode* createChild(SamplerNode* parent, bool tE, Int lvlDiff, Int cmprsdLvl);
		SamplerNode* createChild(SamplerNode* parent, bool tE, Int cmprsdLvl, Int parentIndex, vector<Int> missingIndices, unordered_map<Int, Float> litWts);
		SamplerNode* copy(SamplerNode* other);
		void addParent(SamplerNode* sNode, SamplerNode* newParent, bool tE, Int lvlDiff);
		void addParent(SamplerNode* sNode, SamplerNode* newParent, bool tE, Int parentIndex, vector<Int> missingIndices, unordered_map<Int, Float> litWts);
		void append(SamplerNode* sNode, SamplerNode* other);
		SamplerNode* createSamplerNode(); // unused
		void recycleSamplerNode(SamplerNode*);
		void clearSentinel(Int cmprsdLvl);
		std::pair<std::pair<SamplerNode*,bool>,Int> sampleParent(SamplerNode* nodeSN, vector<double_t> weights, RandomBits *rb);
		std::pair<std::pair<SamplerNode*,bool>,Int> sampleParent(SamplerNode* nodeSN, vector<long double> weights, RandomBits *rb);
		bool noUsedNodes();
		void printSizes();
		
		SamplerNode sentinel;
		
	private:
		NumType *zero, *one;
		SamplerNodeFactory::NUMTYPE SamplerNodeType;
		unordered_set<SamplerNode*> usedNodes; deque<SamplerNode*> freeNodes;
		unordered_set<NumType*> usedRPCs; deque<NumType*> freeRPCs;
		vector<NumType*> samplingArray; NumType *runningTotal, *r;
		vector<double> sA2;
		void ensureSamplingArraySize(Int minSize);
		SamplerNode* getFreeNode(Int numRPCs); // this can return NULL if the free Pool is empty. Callers responsibility to update used pool if creating new node.
		NumType* getRPC(); //unlike getFreeNode, this never returns NULL. It creates a new RPC if needed
		void recycleRPC(NumType*);
		SamplerNodeFactory(SamplerNodeFactory::NUMTYPE SamplerNodeType_);
		SamplerNodeFactory();
    	SamplerNodeFactory(const SamplerNodeFactory &) { }
    	SamplerNodeFactory &operator=(const SamplerNodeFactory &) { return *this; }
		
};
} //end namespace Sampler

#endif


/*

*/