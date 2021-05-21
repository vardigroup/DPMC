#include "sampler/utils.hpp"

#include "../../../lib/cudd-3.0.0/util/util.h"

#include <sstream>
#include <iostream>

using std::cout;
using std::endl; 
using std::string;
using std::stringstream;


SamplerUtils::ADDWrapper::ADDWrapper(ADD add_): add(add_){}

SamplerUtils::ADDWrapper::ADDWrapper() {}


void SamplerUtils::printPerm(DdManager* man){
	printf("Perm:\n");
    for (int i = 0; i<man->size; i++){
        Cudd_ReadPerm(man,i);
        printf("%d ",man->perm[i]);
    }
    printf("\n");
    printf("InvPerm:\n");
    for (int i = 0; i<man->size; i++){
        printf("%d ",man->invperm[i]);
    }
    printf("\nVar IDs:\n");
    for (int i = 0; i<man->size; i++){
        printf("%d ",man->vars[i]->index);
    }
    printf("\n");
}

void SamplerUtils::printDoubleLine(){
	cout<<"===================================================================================\n";
}

string SamplerUtils::printTimeTaken(string forWhat, Float timeTaken, Int numNewLines){
	stringstream s;
	s << "Time Taken "<<forWhat<<":"<<timeTaken<<" ";
	cout<<"Time Taken "<<forWhat<<":"<<timeTaken<<" "<<std::flush;
	for(Int i = 0; i<numNewLines; i++){
		cout<<"\n";
	}
	return s.str();
}

int SamplerUtils::Cudd_StringSummary(
  DdManager * dd /**< manager */,
  DdNode * f /**< %DD to be summarized */,
  int n, /**< number of variables for minterm computation */
  string* out /** output string*/)
{
	int mode = 0;/**We force integer (0) mode instead of exponential (1) so that we can use string
	instead of printing to stdout */
    DdNode *azero, *bzero;
    int	nodes, leaves, digits;
    int retval = 1;
    DdApaNumber count;

    if (dd == NULL) {
        return(0);
    }
    if (f == NULL) {
	//(void) fprintf(dd->out,": is the NULL DD\n");
	//(void) fflush(dd->out);
	out->append(": is the NULL DD\n");
        dd->errorCode = CUDD_INVALID_ARG;
	return(0);
    }
    azero = DD_ZERO(dd);
    bzero = Cudd_Not(DD_ONE(dd));
    if (f == azero || f == bzero){
        //(void) fprintf(dd->out,": is the zero DD\n");
        //(void) fflush(dd->out);
		out->append(": is the zero DD\n");
        return(1);
    }
    nodes = Cudd_DagSize(f);
    if (nodes == CUDD_OUT_OF_MEM) retval = 0;
    leaves = Cudd_CountLeaves(f);
    if (leaves == CUDD_OUT_OF_MEM) retval = 0;
    //(void) fprintf(dd->out,": %d nodes %d leaves ", nodes, leaves);
	std::ostringstream stringStream;
	stringStream << ": "<<nodes<<" nodes "<<leaves<<" leaves ";
    out->append(stringStream.str());
	count = Cudd_ApaCountMinterm(dd, f, n, &digits);
    if (count == NULL) {
	retval = 0;
    } else{
        char* mintermStr = Cudd_ApaStringDecimal(digits, count);
		out->append(mintermStr);
		retval = 1;
    }
    FREE(count);
    //(void) fprintf(dd->out, " minterms\n");
    //(void) fflush(dd->out);
    out->append(" minterms\n");
	return(retval);
} /* end of Cudd_PrintSummary */