#include "sampler/SamplerNode.hpp"
#include <iostream>

Sampler::SamplerNode::SamplerNode( vector<SamplerNode*> parents_, vector<NumType*> rootPathCounts_, NumType* rootPathCountTotal_, Int compressedLevel_,
	vector<bool> thenElse_): 
compressedLevel(compressedLevel_), rootPathCounts(rootPathCounts_), rootPathCountTotal(rootPathCountTotal_), parents(parents_), 
	thenElse(thenElse_){}
Sampler::SamplerNode::SamplerNode( SamplerNode* parent_, NumType* rootPathCount_, NumType* rootPathCountTotal_, Int compressedLevel_,
	bool thenElse_): 
compressedLevel(compressedLevel_), rootPathCounts(1,rootPathCount_), rootPathCountTotal(rootPathCountTotal_), parents(1,parent_), 
	thenElse(1,thenElse_){}
Sampler::SamplerNode::SamplerNode(NumType* rootPathCountTotal_, Int compressedLevel_):
	compressedLevel(compressedLevel_), rootPathCountTotal(rootPathCountTotal_){}
Sampler::SamplerNode::SamplerNode(){}
Sampler::SamplerNode::SamplerNode(const SamplerNode& other){
		std::cout<<"Something is wrong. SamplerNode's copy constructor should not be used. Exiting..\n";
		exit(1);
}
Sampler::SamplerNode::~SamplerNode(){}

Int Sampler::SamplerNode::getNumParents(){
	//std::cout<<parents.size()<<" "<<thenElse.size()<<" "<<rootPathCounts.size()<<"\n";
	return parents.size();
}

Int Sampler::SamplerNode::getCompressedLevel(){
	return compressedLevel;
}

void Sampler::SamplerNode::printParentCmpLvls(){
	//printf("parent cmprsdlvls are:\n");
	for(auto& parent: parents){
		std::cout<<parent->compressedLevel<<" "<<std::flush;
	}
	std::cout<<"\n";
}