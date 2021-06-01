#include "sampler/SamplerNodeFactory.hpp"
#include <cassert>
#include <algorithm>

using std::cout; using std::endl;
using Sampler::SamplerNode;

bool compareNumType(NumType*a, NumType* b){return a->lessThan(b);}

Sampler::SamplerNodeFactory* Sampler::SamplerNodeFactory::getFactory(Int memLimit, SamplerNodeFactory::NUMTYPE SamplerNodeType_){
	static SamplerNodeFactory instance(SamplerNodeType_);
	return &instance;
}
void Sampler::SamplerNodeFactory::forgetAllUsedNodes(){
	usedNodes.clear();
	usedRPCs.clear();
}
void Sampler::SamplerNodeFactory::recycleAllUsedNodes(){
	freeNodes.insert(freeNodes.end(),usedNodes.begin(),usedNodes.end());
	freeRPCs.insert(freeRPCs.end(),usedRPCs.begin(),usedRPCs.end());
	usedNodes.clear();
	usedRPCs.clear();
	sentinel.rootPathCounts.clear(); // sentinel rpcs get recycled above. so need to clear inorder to not recycle twice
}
SamplerNode* Sampler::SamplerNodeFactory::createSamplerNode(){
	cout<<"Not implemented createSamplerNode in snFactory!\n";
	return NULL;
}
void Sampler::SamplerNodeFactory::recycleSamplerNode(SamplerNode* node){
	//assert(usedNodes.erase(node));
	usedNodes.erase(node);
	//cout<<"Recycling rpcs in recSamplerNode\n";
	for(auto rpc: node->rootPathCounts){
		recycleRPC(rpc);
	}
	//parents, thenelse, rpcs vectors to be cleared in get-node functions
	freeNodes.push_back(node);
}

void Sampler::SamplerNodeFactory::recycleRPC(NumType* rpc){
	//assert(usedRPCs.erase(rpc));
	usedRPCs.erase(rpc);
	freeRPCs.push_back(rpc);
}

SamplerNode* Sampler::SamplerNodeFactory::createRoot(Int cmprsdLvl){
	SamplerNode* tmp = getFreeNode(0);
	if(tmp!=NULL){
		tmp->compressedLevel = cmprsdLvl;
		tmp->rootPathCountTotal->setPowerOfTwo(cmprsdLvl); //technically can be set to 1
	} else{
		NumType* rpcT = one->multiplyByPowerOfTwo(cmprsdLvl);
		tmp = new SamplerNode(rpcT,cmprsdLvl);
	}
	usedNodes.insert(tmp);
	return tmp;
}

SamplerNode* Sampler::SamplerNodeFactory::createRoot(Int cmprsdLvl, vector<Int> missingIndices, unordered_map<Int, Float> litWts){
	SamplerNode* tmp = getFreeNode(0);
	if(tmp!=NULL){
		tmp->compressedLevel = cmprsdLvl;
		Float factor = 1;
		for (auto ind : missingIndices){
			factor *= litWts[ind] + litWts[-ind];
		}
		tmp->rootPathCountTotal->setLongDouble(factor);
	} else{
		Float factor = 1;
		for (auto ind : missingIndices){
			factor *= litWts[ind] + litWts[-ind];
		}
		NumType* rpcT = one->multiplyByLongDouble(factor);
		tmp = new SamplerNode(rpcT,cmprsdLvl);
	}
	usedNodes.insert(tmp);
	return tmp;
}

SamplerNode* Sampler::SamplerNodeFactory::createChild(SamplerNode* parent, bool tE, Int lvlDiff, Int cmprsdLvl){
	SamplerNode* tmp = getFreeNode(1);
	if(tmp!=NULL){
		tmp->parents[0] = parent; tmp->thenElse[0]=tE;
		tmp->compressedLevel = cmprsdLvl;
		tmp->rootPathCountTotal->set(parent->rootPathCountTotal);
		tmp->rootPathCountTotal->multiplyByPowerOfTwoSelf(lvlDiff);
		tmp->rootPathCounts[0]->set(tmp->rootPathCountTotal); // rpcs should have exactly 1 element
	} else{
		NumType* rpc = getRPC();
		rpc->set(parent->rootPathCountTotal);
		rpc->multiplyByPowerOfTwoSelf(lvlDiff);
		tmp = new SamplerNode(parent,rpc,rpc->copy(),cmprsdLvl,tE); //rpcTotal is not tracked by rpcPool. Therefore need to copy
	}
	usedNodes.insert(tmp);
	return tmp;
}

SamplerNode* Sampler::SamplerNodeFactory::createChild(SamplerNode* parent, bool tE, Int cmprsdLvl, Int parentIndex, vector<Int> missingIndices, unordered_map<Int, Float> litWts){
	SamplerNode* tmp = getFreeNode(1);
	if(tmp!=NULL){
		tmp->parents[0] = parent; tmp->thenElse[0]=tE;
		tmp->compressedLevel = cmprsdLvl;
		tmp->rootPathCountTotal->set(parent->rootPathCountTotal);
		Float factor = tE? litWts[parentIndex]: litWts[-parentIndex];
		for (auto ind : missingIndices){
			factor *= litWts[ind] + litWts[-ind];
		}
		tmp->rootPathCountTotal->multiplyByLongDoubleSelf(factor);
		tmp->rootPathCounts[0]->set(tmp->rootPathCountTotal); // rpcs should have exactly 1 element
	} else{
		Float factor = tE? litWts[parentIndex]: litWts[-parentIndex];
		for (auto ind : missingIndices){
			factor *= litWts[ind] + litWts[-ind];
		}
		NumType* rpc = getRPC();
		rpc->set(parent->rootPathCountTotal);
		rpc->multiplyByLongDoubleSelf(factor);
		tmp = new SamplerNode(parent,rpc,rpc->copy(),cmprsdLvl,tE); //rpcTotal is not tracked by rpcPool. Therefore need to copy
	}
	usedNodes.insert(tmp);
	return tmp;
}

SamplerNode* Sampler::SamplerNodeFactory::copy(SamplerNode* other){
	SamplerNode* tmp = getFreeNode(0); // since have to copy anyway
	vector<NumType*> rpcs;
	for (auto rpc: other->rootPathCounts){
		NumType* newRPC = getRPC();
		newRPC->set(rpc);
		rpcs.push_back(newRPC);
	}
	if(tmp!=NULL){
		tmp->parents=other->parents; tmp->thenElse=other->thenElse;
		
		tmp->compressedLevel = other->compressedLevel;
		tmp->rootPathCountTotal->set(other->rootPathCountTotal);
		tmp->rootPathCounts = rpcs; //shallow copy sufficient
	} else{
		tmp = new SamplerNode(other->parents,rpcs,other->rootPathCountTotal->copy(),other->compressedLevel,other->thenElse);
	}
	usedNodes.insert(tmp);
	return tmp;
}

void Sampler::SamplerNodeFactory::addParent(SamplerNode* sNode, SamplerNode* newParent, bool tE, Int lvlDiff){
	sNode->parents.push_back(newParent);
	sNode->thenElse.push_back(tE);
	NumType* rpc = getRPC();
	rpc->set(newParent->rootPathCountTotal);
	rpc->multiplyByPowerOfTwoSelf(lvlDiff);
	sNode->rootPathCounts.push_back(rpc);
	sNode->rootPathCountTotal->addSelf(rpc);
}

void Sampler::SamplerNodeFactory::addParent(SamplerNode* sNode, SamplerNode* newParent, bool tE, Int parentIndex, vector<Int> missingIndices, unordered_map<Int, Float> litWts){
	sNode->parents.push_back(newParent);
	sNode->thenElse.push_back(tE);
	Float factor = tE? litWts[parentIndex]: litWts[-parentIndex];
	for (auto ind : missingIndices){
		factor *= litWts[ind] + litWts[-ind];
	}
	NumType* rpc = getRPC();
	rpc->set(newParent->rootPathCountTotal);
	rpc->multiplyByLongDoubleSelf(factor);
	sNode->rootPathCounts.push_back(rpc);
	sNode->rootPathCountTotal->addSelf(rpc);
}

void Sampler::SamplerNodeFactory::append(SamplerNode* sNode, SamplerNode* other){
	sNode->parents.insert(sNode->parents.end(),other->parents.begin(),other->parents.end());
	sNode->thenElse.insert(sNode->thenElse.end(),other->thenElse.begin(),other->thenElse.end());
	for(auto rpc : other->rootPathCounts){
		NumType* newRPC = getRPC();
		newRPC->set(rpc);
		sNode->rootPathCounts.push_back(newRPC);
	}
	sNode->rootPathCountTotal->addSelf(other->rootPathCountTotal);
}

SamplerNode* Sampler::SamplerNodeFactory::getFreeNode(Int numRPCs){
	if (freeNodes.empty()) return NULL;
	else {
		SamplerNode* front = freeNodes.front();
		freeNodes.pop_front();
		front->parents.resize(numRPCs);
		front->thenElse.resize(numRPCs);
		front->rootPathCounts.clear(); //safe to do so as rpcs were returned to free pool when the node was recycled
		for (Int i = 0; i<numRPCs; i++){
			front->rootPathCounts.push_back(getRPC());
		}
		return front;
	}
}

NumType* Sampler::SamplerNodeFactory::getRPC(){
	NumType* newRPC;
	if (freeRPCs.empty()) {
		newRPC = one->copy();
		//cout<<"free RPCs empty!\n";
	} else{
		newRPC = freeRPCs.front();
		freeRPCs.pop_front();
	}
	//assert(usedRPCs.insert(newRPC).second);
	usedRPCs.insert(newRPC);
	return (newRPC);
}

void Sampler::SamplerNodeFactory::clearSentinel(Int cmprsdLvl){
	sentinel.parents.clear(); sentinel.thenElse.clear(); 
	//cout<<"Recycling rpc in clrSentinel\n";
	for (auto rpc: sentinel.rootPathCounts){
		recycleRPC(rpc);
	}
	sentinel.rootPathCounts.clear();
	sentinel.rootPathCountTotal->set(zero);
	sentinel.compressedLevel = cmprsdLvl;
}

void Sampler::SamplerNodeFactory::ensureSamplingArraySize(Int minSize){
	Int currSize = samplingArray.size();
	while(currSize<minSize){
		samplingArray.push_back(one->copy());
		//sA2.push_back(0); 
		currSize++;
	}
}

std::pair<std::pair<SamplerNode*,bool>,Int> Sampler::SamplerNodeFactory::sampleParent(SamplerNode* nodeSN, vector<double_t> weights, RandomBits *rb){
	ensureSamplingArraySize(nodeSN->parents.size());
	runningTotal->set(zero);
	//assert(nodeSN->rootPathCounts.size()==nodeSN->parents.size());
	{ //braces to limit definition of i. i might be used later in loops and don't want double definition
		Int i = 0;
		for (auto rpc : nodeSN->rootPathCounts){
			samplingArray[i]->set(rpc);
			if(!weights.empty()) samplingArray[i]->multiplyByDoubleSelf(weights.at(i));
			//assert(samplingArray[i]->isPositive());
			//no need to separately do lvlDiff as the rpcs have incorporated the lvldiff already
			runningTotal->addSelf(samplingArray[i]);
			samplingArray[i]->set(runningTotal);
			i++;
		}
	}
	r->setRandom(runningTotal,rb);
	
	//lower_bound returns position of first element in array that is greater than or equal to r
	vector<NumType*>::iterator pos = std::lower_bound(samplingArray.begin(),samplingArray.begin()+nodeSN->parents.size(),r, compareNumType);
	Int low = pos - samplingArray.begin();
	return std::make_pair(std::make_pair(nodeSN->parents.at(low),nodeSN->thenElse.at(low)),low);
}

std::pair<std::pair<SamplerNode*,bool>,Int> Sampler::SamplerNodeFactory::sampleParent(SamplerNode* nodeSN, vector<long double> weights, RandomBits *rb){
	ensureSamplingArraySize(nodeSN->parents.size());
	runningTotal->set(zero);
	//cout<<nodeSN->rootPathCounts.size()<<" "<<nodeSN->parents.size()<<" "<<nodeSN->thenElse.size()<<"\n";
	//assert(nodeSN->rootPathCounts.size()==nodeSN->parents.size());
	{ //braces to limit definition of i. i might be used later in loops and don't want double definition
		Int i = 0;
		for (auto rpc : nodeSN->rootPathCounts){
			samplingArray[i]->set(rpc);
			if(!weights.empty()) samplingArray[i]->multiplyByLongDoubleSelf(weights.at(i));
			//assert(samplingArray[i]->isPositive());
			//no need to separately do lvlDiff as the rpcs have incorporated the lvldiff already
			runningTotal->addSelf(samplingArray[i]);
			samplingArray[i]->set(runningTotal);
			i++;
		}
	}
	r->setRandom(runningTotal,rb);
	
	//lower_bound returns position of first element in array that is greater than or equal to r
	vector<NumType*>::iterator pos = std::lower_bound(samplingArray.begin(),samplingArray.begin()+nodeSN->parents.size(),r, compareNumType);
	Int low = pos - samplingArray.begin();
	return std::make_pair(std::make_pair(nodeSN->parents.at(low),nodeSN->thenElse.at(low)),low);
}

bool Sampler::SamplerNodeFactory::noUsedNodes(){
	return usedNodes.empty();
}

void Sampler::SamplerNodeFactory::printSizes(){
	cout<<"SamplerNodes: free "<<freeNodes.size()<<" used "<<usedNodes.size()<<"\n";
	cout<<"RPCs: free "<<freeRPCs.size()<<" used "<<usedRPCs.size()<<"\n";
}

Sampler::SamplerNodeFactory::SamplerNodeFactory(SamplerNodeFactory::NUMTYPE SamplerNodeType_): SamplerNodeType(SamplerNodeType_),
	sentinel(NULL,1){
	
	switch (SamplerNodeType){
		case SamplerNodeFactory::dbl :
			zero = new DoubleNumType(0.0);
			one = new DoubleNumType(1.0);
			break;
		case SamplerNodeFactory::long_dbl :
			zero = new LongDoubleNumType(0.0);
			one = new LongDoubleNumType(1.0);
			break;
		case SamplerNodeFactory::gmpz :
			zero = new MPZNumType(0);
			one = new MPZNumType(1);
			break;
		default:
			cout<<"Unrecognized SamplerNode Type: "<<SamplerNodeType<<" exiting..\n";
			exit(0);
			break;
	}
	sentinel.rootPathCountTotal = zero->copy();
	runningTotal = zero->copy();
	r = zero->copy();
}