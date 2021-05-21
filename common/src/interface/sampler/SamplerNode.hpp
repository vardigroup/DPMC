#ifndef SAMPLERNODE_H_
#define SAMPLERNODE_H_

#include <vector>
#include <cmath>
#include <unordered_map>
#include "sampler/NumType.hpp"

using std::vector;
using std::unordered_map;

namespace Sampler{
class SamplerNode{
	public:	
		Int getCompressedLevel();
		void printParentCmpLvls();
		Int getNumParents();
	private:
		friend class SamplerNodeFactory;
		//cant have a vector of references by C++ design. Can use reference_wrapper, but will go with pointers instead
		vector<SamplerNode*> parents;
		vector<bool> thenElse; //stores whether the current node is reached through then / else edge from parent
		// thenElse can enable DdNodes in Cudd corresponding to the SamplingStructure to be deleted / dealloced
		//after the SamplingStructure has been constructed, for saving space. NO! this might not work since the DdNodes
		//may be shared with other ADDs where they are part of s or s''. Despite that, it may not be a big overhead to
		//store thenElse since vector of bools is compressed
		vector<NumType*> rootPathCounts;
		NumType* rootPathCountTotal;
		Int compressedLevel;

		SamplerNode( vector<SamplerNode*> parents_, vector<NumType*> rootPathCounts_, NumType* rootPathCountTotal, Int compressedLevel_,
			vector<bool> thenElse_);
		SamplerNode( SamplerNode* parent_, NumType* rootPathCounts_, NumType* rootPathCountTotal, Int compressedLevel_,
			bool thenElse_);
		SamplerNode(NumType* rootPathCountTotal, Int compressedLevel_);
		SamplerNode();
		SamplerNode(const SamplerNode& other);
		~SamplerNode();
};
} // end namespace Sampler

#endif
