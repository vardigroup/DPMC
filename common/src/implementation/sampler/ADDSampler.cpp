#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <random>
#include <cassert>

#include "sampler/ADDSampler.hpp"

using std::cout;
using std::endl;
using std::sort;
using Sampler::Asmt;
using Sampler::ADDSampler;

void Asmt::set(bool bit, Int i){
	assert(bits.at(i)==-1); //sampling is generally not the bottleneck so leaving asserts in place
	bits[i] = bit;
}
void Asmt::setNoCheck(bool bit, Int i){
	printf("\nSet called on state %p at pos %ld!\n",this,i);
	bits[i] = bit;
}

void Asmt::printAsmt(){
	cout<<"Asmt:\n";
	for(Int i = 0; i<bits.size();i++){
		printf("%d",bits[i]);
	}
	cout<<"\n";
}

bool Asmt::get(Int i){
	assert(bits.at(i)!=-1); //sampling is generally not the bottleneck so leaving asserts in place
	return bits[i]==1;
}
void Asmt::clear(){
	for (Int i = 0; i<bits.size();i++){
		bits[i] = -1;
	}
}

ADDSampler::ADDSampler(JoinNode *root_, Cudd mgr_ , Int nTotalVars_, Int nApparentVars_, unordered_map<Int,Int> c2DVarMap, vector<Int> d2CVarMap,
	unordered_map<Int, Float> litWeights_, Set<Int> freeVars_,	bool checkAsmts_, Float startTime_): cnfVarToDdVarMap(c2DVarMap), ddVarToCnfVarMap(d2CVarMap), 
	litWeights(litWeights_), freeVars(freeVars_), checkAsmts(checkAsmts_), startTime(startTime_), jtRoot(root_), mgr(mgr_), nTotalVars(nTotalVars_),
	nApparentVars(nApparentVars_){
	
	rb.SeedEngine();
	rb.SeedEngine2();
	snFactory = SamplerNodeFactory::getFactory(0,SamplerNodeFactory::long_dbl);
	for (Int i = 0; i<nApparentVars; i++){
		assert(mgr.ReadPerm(i)==i);
		//cout<<i<<" "<<mgr.ReadPerm(i)<<"\n";
		//this is because dpmc maintains mapping between cnfvars and ddvars instead of using cudd_addpermute
		//so index and levels of all ddvars should be same. 
		//this is different from tracesampler which used permute
		//therefore all code here is written under assumption that index and levels are same for all ddvars
	}
	assert(nApparentVars+freeVars.size()==nTotalVars);
	numAssigned = 0;
}

void ADDSampler::buildDataStructures(){
	t = new Asmt(nTotalVars+1); //cnfvarids are indexed from 1.
	//TODO: Disable auto reordering
	//Cudd_AutodynDisable(a.dd);
	//printComment("#nodes in JoinTree:"+to_string(jtRoot->getNodeCount()));
	printComment("Building aux structures..  ",0,0);
	createAuxStructures(jtRoot);
	printComment("Built all aux structures!",0,1,false);
	//allAuxStructsTime = cpuTimeTotal();
	//SamplerUtils::printTimeTaken("Construct All Aux Structs",allAuxStructsTime - startTime,2);
	printComment("Building sampling DAGs..  ",0,0);
	createSamplingDAGs(jtRoot);
	printComment("Built sampling DAGs!",0,1,false);
	//allDAGsTime = cpuTimeTotal();
	//SamplerUtils::printTimeTaken("Construct All Sampling DAGs",allDAGsTime - allAuxStructsTime,2);
	printComment("Finished building all Datastructures!");
}

void ADDSampler::createAuxStructure(JoinNode* jNode){
	//compressedPerm only includes levels for variables that are to be sampled, i.e. the vars in projectedVars of joinnode
	//that means that for root add, it will include all vars in the add
	class Indexer{
		public:
		Int level;
		Int index;
		Indexer(Int index_, Int level_): level(level_), index(index_){}
		bool operator < (const Indexer& str) const{
        	return (level < str.level);
    	}
	};
	Set<Int> pVars = jNode->getProjectableCnfVars();
	Int numPVars = pVars.size();
	SamplerUtils::ADDWrapper& a = jNode->getNodeDD();
	if (jNode==jtRoot){
		a.precompileSampleDAG = true;  // no assignments at root so can precompile
	} else{
		a.precompileSampleDAG = false; //cant precompile unless restrictions on variable ordering. Usually not efficient to
		//have such restrictions that enable precompiling so setting to false. (restrictions would be having all samplevars
		//grouped together)
	}
	if(numPVars == 0){
		return;
	}
	vector<Indexer> ind_level_map;
	//generate perm o(n)
	for(auto p: pVars){
		ind_level_map.emplace_back(p,cnfVarToDdVarMap.at(p));
	}
	a.compressedInvPerm = new Int[2*numPVars];
	a.sampleStructDepth = numPVars + 1; //since sample structure should include leaves
	
	//cout<<"Sorting..\n";
	//sort nlogn
	sort(ind_level_map.begin(),ind_level_map.end());
	
	//compress and insert o(n) 
	a.startSampleVarLevel = ind_level_map.at(0).level;
	a.endSampleVarLevel = ind_level_map.at(numPVars-1).level; // changed below for nth add
	for(int i = 0; i< numPVars; i++){
		a.compressedPerm[ind_level_map.at(i).level] = i; //cmprsdperm stores mapping from ddvarid to cmprsdlevl
		a.compressedInvPerm[2*i] = ind_level_map.at(i).index;
		a.compressedInvPerm[2*i+1] = ind_level_map.at(i).level;
	}
	//cmprsdperm from ddvarindex to cmprsdlvl
	//cmprsdinvprm from cmprsdlvl to cnfvarindex,ddvarindx
}

void ADDSampler::createAuxStructures(JoinNode* jNode){
	createAuxStructure(jNode);
	for (auto child: jNode->getChildren()){
		createAuxStructures(child);
	}
}

void ADDSampler::createSamplingDAG(JoinNode* jNode){
	SamplerUtils::ADDWrapper& a = jNode->getNodeDD();
	assert(a.sampleStructDepth > 0);
	vector<unordered_map<DdNode*, SamplerNode*>> sampleHashes(a.sampleStructDepth);
	Int numRoot = 0;
	for (auto& rootPair: a.rootSNs){
		SamplerNode* root = rootPair.second;
		numRoot++;
		sampleHashes.clear();
		sampleHashes.resize(a.sampleStructDepth);
		//cout<<"root level:"<<root->getCompressedLevel()<<" samplestructdepth:"<<a.sampleStructDepth<<"\n";
		sampleHashes.at(root->getCompressedLevel()).emplace(rootPair.first,root);
		for (int i = 0; i<a.sampleStructDepth-1; i++){ //Process the last level separately
			for (auto& hashedPair : sampleHashes.at(i)){
				DdNode* node = hashedPair.first;
				SamplerNode* nodeSN = hashedPair.second;
				DdNode* t = Cudd_T(node); 
				Int tLevel = a.sampleStructDepth-1; //last lvel
				//cout<<"tindex"<<t->index<<"\n";
				if (a.compressedPerm.find(t->index) != a.compressedPerm.end()){
					tLevel = a.compressedPerm.at(t->index);
				}
				//cout<<"tlevel:"<<tLevel<<"\n";
				std::unordered_map<DdNode*,SamplerNode*>::const_iterator newNode = sampleHashes.at(tLevel).find(t);
				if (newNode == sampleHashes.at(tLevel).end()){
					SamplerNode* tSN = snFactory->createChild(nodeSN,true,tLevel-(i+1),tLevel);
					sampleHashes.at(tLevel).emplace(t,tSN);
				} else{
					SamplerNode* nNode = newNode->second;
					snFactory->addParent(nNode, nodeSN,true,tLevel-(i+1));
				}
			
				DdNode* e = Cudd_E(node); 
				Int eLevel = a.sampleStructDepth-1; //last lvel
				if (a.compressedPerm.find(e->index) != a.compressedPerm.end()){
					eLevel = a.compressedPerm.at(e->index); 
				}
				//cout<<"elevel:"<<eLevel<<"\n";
				std::unordered_map<DdNode*,SamplerNode*>::const_iterator newNode2 = sampleHashes.at(eLevel).find(e);
				if (newNode2 == sampleHashes.at(eLevel).end()){
					SamplerNode* eSN = snFactory->createChild(nodeSN,false,eLevel-(i+1),eLevel);
					sampleHashes.at(eLevel).emplace(e,eSN);
				} else{
					SamplerNode* nNode = newNode2->second;
					snFactory->addParent(nNode,nodeSN,false,eLevel-(i+1));
				}
				//cout<<"Done inner iteration with i="<<i<<"\n";
			}
		}
		for (auto& leaf : sampleHashes[a.sampleStructDepth-1]){
			a.allSamplingPreLeaves[rootPair.first].push_back(leaf);
		}
		if (a.allSamplingPreLeaves[rootPair.first].size()==0){
			cout<<"WARNING: No (pre)leaf found for a rootSN for add.";
			cout<<" samplingStructDepth-1 = "<<a.sampleStructDepth-1<<" compressedVarLevel of rootSN is "
			<<root->getCompressedLevel()<<" sampleHash size at last level is "<<sampleHashes[a.sampleStructDepth-1].size();
			cout<<"isConstNode: "<<Cudd_IsConstant(rootPair.first)<<" Exiting..\n";
			exit(1);
		}
	}
	//cout<<"Finished creating sampling dag!\n";
}

void ADDSampler::createSamplingDAGs_rec(SamplerUtils::ADDWrapper& a, DdNode* currNode, unordered_set<DdNode*>* sampleRootSet){
	if(Cudd_IsConstant(currNode)){
		assert(Cudd_V(currNode)==0); // while recursing to find rootSNs, typically we should not encounter a non-0 
		//leaf without seeing any sample vars
		return;
	}
	Int currLevel = currNode->index; // we assume (following dpmc) that index and levels are same for all ADD vars
	if(currLevel>=a.startSampleVarLevel){
		sampleRootSet->insert(currNode);
	} else {
		DdNode* t = Cudd_T(currNode);
		createSamplingDAGs_rec(a, t, sampleRootSet);
		DdNode* e = Cudd_E(currNode);
		createSamplingDAGs_rec(a, e, sampleRootSet);
	}
}

void ADDSampler::createSamplingDAGs(JoinNode* jNode){
	//cout<<"Creating sampling structure..\n";
	SamplerUtils::ADDWrapper& a = jNode->getNodeDD();
	if(!a.precompileSampleDAG) {
		//cout<<"Not precompiling\n";
	} else{
		unordered_set<DdNode*> sampleRootSet;
		DdNode* currNode = a.add.getNode();
		Int currLevel = currNode->index; // we assume (following dpmc) that index and levels are same for all ADD vars
		if(currLevel>=a.startSampleVarLevel){
			//cout<<"Only one root!\n";
			Int cmprsdLevel = a.sampleStructDepth-1;
			if (a.compressedPerm.find(currLevel) != a.compressedPerm.end()){
				cmprsdLevel = a.compressedPerm.at(currLevel);
			}
			a.rootSNs.emplace(currNode,snFactory->createRoot(cmprsdLevel));
		} else {
			//cout<<"Recursing to find roots..\n";
			DdNode* t = Cudd_T(currNode);
			createSamplingDAGs_rec(a, t, &sampleRootSet);
			DdNode* e = Cudd_E(currNode);
			createSamplingDAGs_rec(a, e, &sampleRootSet);
			for (DdNode* root: sampleRootSet){
				Int cmprsdLevel = a.sampleStructDepth-1;
				if (a.compressedPerm.find(root->index) != a.compressedPerm.end()){
					cmprsdLevel = a.compressedPerm.at(root->index);
				}
				a.rootSNs.emplace(root,snFactory->createRoot(cmprsdLevel));
			}
		}
		//cout<<"Found "<<a.rootSNs.size()<<" roots, creating sampling DAG(s)\n";
		createSamplingDAG(jNode);
		snFactory->forgetAllUsedNodes();
	}
	for (auto child: jNode->getChildren()){
		createSamplingDAGs(child);
	}
}

#define TRAVERSE(_cn_,_cl_,_cond_,_defErrMsg_) \
	{while (_cond_){ \
		/*DBG printf("%p ",_cn_);*/ \
		bool _bit_ = t->get(ddVarToCnfVarMap.at(_cl_)); \
		/*DBG printf("%d ",_bit_);*/ \
		_cn_ = _bit_? Cudd_T(_cn_) : Cudd_E(_cn_); \
		_cl_ = _cn_->index; \
	} /*DBGprintf("\n");*/}

/* for general case, where each add may be different wrt where s' is located and consequently how many sampling 
 * structures 
 * there are, the general skeleton of the next procedure will be 
 * 		propagate intial -- traverse from some starting root to get to a root node which can be searched in a 
 * 							hash table to get the preleaves
 * 		get leaves -- propagate remaining to get the leafset from preleaves
 * 		sample from structure -- the resulting samplestructure is now complete and we can sample s' from it
 * 
 * Depending on which variable ordering is used in the add, some of the above phases may be empty
 * for example if s' is in beginning then propagate initial will be empty.
 * 
 * Can also think of having all s,s',s'' variables interspersed and constructing the samplingstructure on the fly say 
 * for the
 * f_n f_n-1 .. adds which are not going to be sampled very frequently. But this is not accommodated for now.
 */ 
void ADDSampler::sampleFromADD(JoinNode* jNode){
	SamplerUtils::ADDWrapper& a = jNode->getNodeDD();
	ADD tempADD;
	if (!a.precompileSampleDAG){
		assert(snFactory->noUsedNodes());
		tempADD = a.add;
		a.add = a.add.Cofactor(assignedVarsCube);
		assert(a.rootSNs.empty());
		Int currLevel = a.add.getNode()->index;
		Int cmprsdLevel = a.sampleStructDepth-1;
		if (a.compressedPerm.find(currLevel) != a.compressedPerm.end()){
			cmprsdLevel = a.compressedPerm.at(currLevel);
		}
		SamplerNode* nodeSN = snFactory->createRoot(cmprsdLevel);
		a.rootSNs.emplace(a.add.getNode(),nodeSN);
		createSamplingDAG(jNode);
	}
	
	//traverse till level
	Int currLevel = a.add.getNode()->index;  //get absolute level not compressed. index is same as level for dpmc
	DdNode* currNode = a.add.getNode();
	//cout<<"Currlevel:"<<currLevel<<" startsamplevarlevel:"<<a.startSampleVarLevel<<"\n";
	TRAVERSE(currNode,currLevel,currLevel<a.startSampleVarLevel,"Error during intial traverse");
	DdNode* dagRoot = currNode;
	assert(a.rootSNs.find(currNode)!=a.rootSNs.end());
	SamplerNode* dagRootSN = a.rootSNs.at(dagRoot);
	vector<std::pair<DdNode*,SamplerNode*>>& pleaves = a.allSamplingPreLeaves.at(currNode);

	//get leaves from preleaves
	vector<SamplerNode*> temp;
	
	unordered_map<DdNode*, SamplerNode*> sampleHash;
	//cout<<"preleaves size:"<<pleaves.size()<<" "<<std::flush;
	// DBG cout<<"\npLeaf and its parents' cmpLvl:\n"<<std::flush;
	for (auto& pLeafPair: pleaves){
		//for each preleaf, we will find the leaf by traversing. We create a samplerNode for this leaf and make it
		//point to all the parents of the preleaf. Thus conceptually the leaf will replace (ignore) the preleaf
		//and while sampling we will directly go from the leaf to the ancestor (parent)) of the preleaf without
		//encountering the preleaf at all. It maybe possible that a leaf has the same ancestor twice in the 
		//parent array since we are copying all the ancestors from the preleaf. But this is fine -- we are also
		//copying the thenElse array, and the rootPathCounts will take care of sampling the ancestor with the right 
		//weight
		SamplerNode* pLeaf = pLeafPair.second;
		DdNode* currNode = pLeafPair.first;
		currLevel = currNode->index;
		TRAVERSE(currNode,currLevel,!Cudd_IsConstant(currNode),"Error while finding leaves from preleaves.");
		//even if the preleaf was a leaf, we create a new SamplerNode for it below, since we destroy all the
		//SamplerNodes we create for leaves as they are not used again
		if(currNode == dagRoot){
			assert(pleaves.size()==1);
			assert(sampleHash.size()==0);
			assert(Cudd_V(currNode)!=0); // 0 node should never be exclusively reached
			sampleHash.emplace(dagRoot,dagRootSN);
			break;
		}
		if (Cudd_V(currNode)==0) continue; //skip 0 leaves
		std::unordered_map<DdNode*,SamplerNode*>::const_iterator newNode = sampleHash.find(currNode);
		if (newNode == sampleHash.end()){
			// copy all the parents and thenElse, as conceptually, the preleaf will be replaced by the leaf
			SamplerNode* currNodeSN = snFactory->copy(pLeaf);//WARNING: currNodeSN has the same ddnode as pleaf and not currNode
			sampleHash.emplace(currNode,currNodeSN);
		} else{
			//cout<<"Appending!\n";
			SamplerNode* nNode = newNode->second;
			snFactory->append(nNode,pLeaf);
		}
	}
	if (sampleHash.size()==0){
		cout<<"WARNING: No leaf nonzero leaf found after final traverse for add";
		cout<<" samplingStructDepth-1 = "<<a.sampleStructDepth-1<<" compressedVarLevel of rootSN is "
		<<(a.rootSNs.find(dagRoot))->second->getCompressedLevel()<<" dagRoot address is:"<<dagRoot;
		cout<<" preleavessize:"<<pleaves.size()<<" dagRoot isConstNode: "<<Cudd_IsConstant(dagRoot)<<" Exiting..\n";
		exit(1);
	}
	
	snFactory->clearSentinel(a.sampleStructDepth); //leaf level is a.sampleStructDepth-1 so for sentinel level +1 
	vector<Float> weights;
	for (auto& leaf : sampleHash){
		weights.push_back(Cudd_V(leaf.first));
		snFactory->addParent(&(snFactory->sentinel),leaf.second,false,0);
	}
	
	//sample from samplestructure
	//first sample a leaf
	std::pair<std::pair<SamplerNode*, bool>,Int> parentSample = snFactory->sampleParent(&(snFactory->sentinel),weights,&rb);
	SamplerNode* currNodeSN = parentSample.first.first;
	assert(weights.at(parentSample.second)!=0);
	Int cnCmprsdLvl = currNodeSN->getCompressedLevel();
	//iteratively sample parents
	while(cnCmprsdLvl>0){
		//named chosenAncestor to emphasize that there might be intermediate skipped nodes, 
		//that we need to explicitly sample
		SamplerNode* chosenAncestor;
		Int caCmprsdLvl = -1; //compressedLevel for chosenAncestor, initially set to -1
		
		if(currNodeSN != dagRootSN){
			if (currNodeSN->getNumParents()==0){
				cout<<"Reached node without parents but not dagroot.\n";
				cout<<currNodeSN->getCompressedLevel()<<" "<<dagRootSN->getCompressedLevel()<<"\n";
				exit(1);
			}
			auto parentSample = snFactory->sampleParent(currNodeSN,vector<Float>(),&rb);;
			chosenAncestor = parentSample.first.first;
			caCmprsdLvl = chosenAncestor->getCompressedLevel();
			//set the state value for chosen ancestor
			bool tE = parentSample.first.second;
			Int cnfVarIndex = a.compressedInvPerm[2*caCmprsdLvl];
			Int ddVarIndex = a.compressedInvPerm[2*caCmprsdLvl+1];
			t->set(tE,cnfVarIndex);
			assignedVarsCube = tE? assignedVarsCube.Times(mgr.addVar(ddVarIndex)) : assignedVarsCube.Times(mgr.addVar(ddVarIndex).Cmpl());
			numAssigned++;
			currNodeSN = chosenAncestor;
		} else{}
		//sample skipped nodes/vars, if any
		//we have already sampled a bit for chosenAncestorLevel. currnodesnlevel would have been smapled in prev round
		for(int i = caCmprsdLvl+1; i<cnCmprsdLvl; i++){
			Int cnfVarIndex = a.compressedInvPerm[2*i];
			Int ddVarIndex = a.compressedInvPerm[2*i+1];
			bool bit = rb.generateWeightedRandomBit(litWeights[cnfVarIndex],litWeights[-cnfVarIndex]);
			t->set(bit,cnfVarIndex); 
			assignedVarsCube = bit? assignedVarsCube.Times(mgr.addVar(ddVarIndex)) : assignedVarsCube.Times(mgr.addVar(ddVarIndex).Cmpl());
			numAssigned++;
		}
		cnCmprsdLvl = caCmprsdLvl;	
	}
	//destroy / delete dynamically allocated objects
	for(auto& leaf: sampleHash){snFactory->recycleSamplerNode(leaf.second);}
	if(!a.precompileSampleDAG){
		a.add = tempADD;
		a.rootSNs.clear();
		snFactory->recycleAllUsedNodes();
		a.allSamplingPreLeaves.clear();
	}
	/*
	NOTE: Cant delete DDnodes after sampling structure is created as DdNodes may be shared among adds and maybe part
	of s or s'' in a different add. 
	*/
}

Asmt& ADDSampler::drawSample(){
	t->clear();
	assignedVarsCube = mgr.addOne();
	numAssigned = 0;
	for(auto fVar : freeVars){
		bool bit = rb.generateWeightedRandomBit(litWeights[fVar],litWeights[-fVar]);
		t->set(bit, fVar);
		//freevars dont appear in DDs. so no need to add to assignedvars
		//Int ddVarIndex = cnfVarToDdVarMap.at(fVar);
		//assignedVarsCube = bit? assignedVarsCube.Times(mgr.addVar(ddVarIndex)) : assignedVarsCube.Times(mgr.addVar(ddVarIndex).Cmpl());
	}
	//cout<<"Set all freeVars!\n";	
	drawSample_rec(jtRoot);
	/*if (checkAsmts){
		if(!checkAsmt()){
			cout<<"Sampled asmt failed check!\n";
			t->printAsmt();
			cout<<"Exiting..\n";
			exit(1);
		}
	}*/
	//cout<<"Sample drawn successfully!\n";
	assert(numAssigned==nApparentVars);
	return *t;
}

void ADDSampler::drawSample_rec(JoinNode* jNode){
	if (jNode->getProjectableCnfVars().size()==0){
		return;
	}
	//cout<<"starting sampling from add\n";
	sampleFromADD(jNode);
	Float checkVal = getAsmtVal(jNode->getNodeDD());
	if (checkVal == 0.0){
		cout<<"While checking sample for add reached leaf with value "<<checkVal<<". Exiting..\n";
		exit(1);
	} else {
		//cout<<"SampleADD passed!\n";
	}
	for(auto child: jNode->getChildren()){
		drawSample_rec(child);
	} 
}

Float ADDSampler::getAsmtVal(SamplerUtils::ADDWrapper& a){
	DdNode* currNode = a.add.getNode();
	Int currLevel = currNode->index;
	TRAVERSE(currNode,currLevel,!Cudd_IsConstant(currNode),"Error during checkSample traverse");
	return (Cudd_V(currNode));
}

bool ADDSampler::checkAsmt(){
	if (getAsmtVal(jtRoot->getNodeDD())==0){
		return false;
	}
	for(auto child: jtRoot->getChildren()){
		if (!checkAsmt_rec(child)){
			return false;
		}
	} 
	return true;
}

bool ADDSampler::checkAsmt_rec(JoinNode* jNode){
	if (getAsmtVal(jNode->getNodeDD())==0){
		return false;
	}
	for(auto child: jNode->getChildren()){
		if (!checkAsmt_rec(child)){
			return false;
		}
	} 
	return true;
}

void ADDSampler::writeAsmtToFile(FILE* ofp){
	for(Int i = 1; i<nTotalVars+1; i++){
		fprintf(ofp,"%d ",t->get(i));
	}
	fprintf(ofp,"\n");
	static Int cnt = 0;
	if(cnt % 5000 == 0){
		snFactory->printSizes();
	}
	cnt ++;
}