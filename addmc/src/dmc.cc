#include "dmc.hh"
#include "util/util.h"
/* global vars ============================================================== */

bool existRandom;
string ddPackage;
bool logCounting;
Float logBound;
string thresholdModel;
bool satSolverPruning;
Int maximizerFormat;
bool maximizerVerification;
bool substitutionMaximization;
Int threadCount;
Int threadSliceCount;
Float memSensitivity;
Float maxMem;
string joinPriority;
Int verboseJoinTree;
Int verboseProfiling;

Int dotFileIndex = 1;

Int dynVarOrdering;

Int satFilter = 0;
/* classes for processing join trees ======================================== */

/* class JoinTree =========================================================== */

JoinNode* JoinTree::getJoinNode(Int nodeIndex) const {
  if (joinTerminals.contains(nodeIndex)) {
    return joinTerminals.at(nodeIndex);
  }
  return joinNonterminals.at(nodeIndex);
}

JoinNonterminal* JoinTree::getJoinRoot() const {
  return joinNonterminals.at(declaredNodeCount - 1);
}

void JoinTree::printTree() const {
  cout << "c p " << JOIN_TREE_WORD << " " << declaredVarCount << " " << declaredClauseCount << " " << declaredNodeCount << "\n";
  getJoinRoot()->printSubtree("c ");
}

JoinTree::JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount) {
  this->declaredVarCount = declaredVarCount;
  this->declaredClauseCount = declaredClauseCount;
  this->declaredNodeCount = declaredNodeCount;
}

/* class JoinTreeProcessor ================================================== */

Int JoinTreeProcessor::plannerPid = MIN_INT;
JoinTree* JoinTreeProcessor::joinTree = nullptr;
JoinTree* JoinTreeProcessor::backupJoinTree = nullptr;

void JoinTreeProcessor::killPlanner() {
  if (plannerPid == MIN_INT) {
    cout << WARNING << "found no pid for planner process\n";
  }
  else if (kill(plannerPid, SIGKILL) == 0) {
    cout << "c killed planner process with pid " << plannerPid << "\n";
  }
  else {
    // cout << WARNING << "failed to kill planner process with pid " << plannerPid << "\n";
  }
}

void JoinTreeProcessor::handleSigAlrm(int signal) {
  assert(signal == SIGALRM);
  cout << "c received SIGALRM after " << util::getDuration(toolStartPoint) << "s\n";

  if (joinTree == nullptr && backupJoinTree == nullptr) {
    cout << "c found no join tree yet; will wait for first join tree then kill planner\n";
  }
  else {
    cout << "c found join tree; killing planner\n";
    killPlanner();
  }
}

bool JoinTreeProcessor::hasDisarmedTimer() {
  struct itimerval curr_value;
  getitimer(ITIMER_REAL, &curr_value);

  return curr_value.it_value.tv_sec == 0 && curr_value.it_value.tv_usec == 0 && curr_value.it_interval.tv_sec == 0 && curr_value.it_interval.tv_usec == 0;
}

void JoinTreeProcessor::setTimer(Float seconds) {
  assert(seconds >= 0);

  Int secs = seconds;
  Int usecs = (seconds - secs) * 1000000;

  struct itimerval new_value;
  new_value.it_value.tv_sec = secs;
  new_value.it_value.tv_usec = usecs;
  new_value.it_interval.tv_sec = 0;
  new_value.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &new_value, nullptr);
}

void JoinTreeProcessor::armTimer(Float seconds) {
  assert(seconds >= 0);
  signal(SIGALRM, handleSigAlrm);
  setTimer(seconds);
}

void JoinTreeProcessor::disarmTimer() {
  setTimer(0);
  cout << "c disarmed timer\n";
}

const JoinNonterminal* JoinTreeProcessor::getJoinTreeRoot() const {
  return joinTree->getJoinRoot();
}

void JoinTreeProcessor::processCommentLine(const vector<string>& words) {
  if (words.size() == 3) {
    string key = words.at(1);
    string val = words.at(2);
    if (key == "pid") {
      plannerPid = stoll(val);
    }
    else if (key == "joinTreeWidth") {
      if (joinTree != nullptr) {
        joinTree->width = stoll(val);
      }
    }
    else if (key == "seconds") {
      if (joinTree != nullptr) {
        joinTree->plannerDuration = stold(val);
      }
    }
  }
}

void JoinTreeProcessor::processProblemLine(const vector<string>& words) {
  if (problemLineIndex != MIN_INT) {
    throw MyError("multiple problem lines: ", problemLineIndex, " and ", lineIndex);
  }
  problemLineIndex = lineIndex;

  if (words.size() != 5) {
    throw MyError("problem line ", lineIndex, " has ", words.size(), " words (should be 5)");
  }

  string jtWord = words.at(1);
  if (jtWord != JOIN_TREE_WORD) {
    throw MyError("expected '", JOIN_TREE_WORD, "'; found '", jtWord, "' | line ", lineIndex);
  }

  Int declaredVarCount = stoll(words.at(2));
  Int declaredClauseCount = stoll(words.at(3));
  Int declaredNodeCount = stoll(words.at(4));

  joinTree = new JoinTree(declaredVarCount, declaredClauseCount, declaredNodeCount);

  for (Int terminalIndex = 0; terminalIndex < declaredClauseCount; terminalIndex++) {
    joinTree->joinTerminals[terminalIndex] = new JoinTerminal();
  }
}

void JoinTreeProcessor::processNonterminalLine(const vector<string>& words) {
  if (problemLineIndex == MIN_INT) {
    string message = "no problem line before internal node | line " + to_string(lineIndex);
    if (joinTreeEndLineIndex != MIN_INT) {
      message += " (previous join tree ends on line " + to_string(joinTreeEndLineIndex) + ")";
    }
    throw MyError(message);
  }

  Int parentIndex = stoll(words.front()) - 1; // 0-indexing
  if (parentIndex < joinTree->declaredClauseCount || parentIndex >= joinTree->declaredNodeCount) {
    throw MyError("wrong internal-node index | line ", lineIndex);
  }

  vector<JoinNode*> children;
  Set<Int> projectionVars;
  bool parsingElimVars = false;
  for (Int i = 1; i < words.size(); i++) {
    string word = words.at(i);
    if (word == ELIM_VARS_WORD) {
      parsingElimVars = true;
    }
    else {
      Int num = stoll(word);
      if (parsingElimVars) {
        Int declaredVarCount = joinTree->declaredVarCount;
        if (num <= 0 || num > declaredVarCount) {
          throw MyError("var '", num, "' inconsistent with declared var count '", declaredVarCount, "' | line ", lineIndex);
        }
        projectionVars.insert(num);
      }
      else {
        Int childIndex = num - 1; // 0-indexing
        if (childIndex < 0 || childIndex >= parentIndex) {
          throw MyError("child '", word, "' wrong | line ", lineIndex);
        }
        children.push_back(joinTree->getJoinNode(childIndex));
      }
    }
  }
  joinTree->joinNonterminals[parentIndex] = new JoinNonterminal(children, projectionVars, parentIndex);
}

void JoinTreeProcessor::finishReadingJoinTree() {
  Int nonterminalCount = joinTree->joinNonterminals.size();
  Int expectedNonterminalCount = joinTree->declaredNodeCount - joinTree->declaredClauseCount;

  if (nonterminalCount < expectedNonterminalCount) {
    cout << WARNING << "missing internal nodes (" << expectedNonterminalCount << " expected, " << nonterminalCount << " found) before current join tree ends on line " << lineIndex << "\n";
  }
  else {
    if (joinTree->width == MIN_INT) {
      joinTree->width = joinTree->getJoinRoot()->getWidth();
    }

    cout << "c processed join tree ending on line " << lineIndex << "\n";
    printRow("joinTreeWidth", joinTree->width);
    printRow("plannerSeconds", joinTree->plannerDuration);

    if (verboseJoinTree >= 1) {
      cout << DASH_LINE;
      joinTree->printTree();
      cout << DASH_LINE;
    }

    joinTreeEndLineIndex = lineIndex;
    backupJoinTree = joinTree;
    JoinNode::resetStaticFields();
  }

  problemLineIndex = MIN_INT;
  joinTree = nullptr;
}

void JoinTreeProcessor::readInputStream() {
  string line;
  while (getline(std::cin, line)) {
    lineIndex++;

    if (verboseJoinTree >= 2) {
      util::printInputLine(line, lineIndex);
    }

    vector<string> words = util::splitInputLine(line);
    if (words.empty()) {}
    else if (words.front() == "=") { // LG's tree separator "="
      if (joinTree != nullptr) {
        finishReadingJoinTree();
      }
      if (hasDisarmedTimer()) { // timer expires before first join tree ends
        break;
      }
    }
    else if (words.front() == "c") { // possibly special comment line
      processCommentLine(words);
    }
    else if (words.front() == "p") { // problem line
      processProblemLine(words);
    }
    else { // nonterminal-node line
      processNonterminalLine(words);
    }
  }

  if (joinTree != nullptr) {
    finishReadingJoinTree();
  }

  if (!hasDisarmedTimer()) { // stdin ends before timer expires
    cout << "c stdin ends before timer expires; disarming timer\n";
    disarmTimer();
  }
}

JoinTreeProcessor::JoinTreeProcessor(Float plannerWaitDuration) {
  cout << "c processing join tree...\n";

  armTimer(plannerWaitDuration);
  cout << "c getting join tree from stdin with " << plannerWaitDuration << "s timer (end input with 'enter' then 'ctrl d')\n";

  readInputStream();

  if (joinTree == nullptr) {
    if (backupJoinTree == nullptr) {
      throw MyError("no join tree before line ", lineIndex);
    }
    joinTree = backupJoinTree;
    JoinNode::restoreStaticFields();
  }

  cout << "c getting join tree from stdin: done\n";

  if (plannerPid != MIN_INT) { // timer expires before first join tree ends
    killPlanner();
  }
}

/* classes for execution ==================================================== */

/* class SatSolver ========================================================== */

bool SatSolver::checkSat(bool exceptionThrowing) {
  lbool satisfiability = cmsSolver.solve();
  if (satisfiability == l_False) {
    if (exceptionThrowing) {
      throw UnsatSolverException();
    }
    return false;
  }
  assert(satisfiability == l_True);
  return true;
}

Assignment SatSolver::getModel() {
  vector<Lit> banLits;
  Assignment model;
  vector<lbool> lbools = cmsSolver.get_model();
  for (Int i = 0; i < lbools.size(); i++) {
    Int cnfVar = i + 1;
    bool val = true;
    lbool b = lbools.at(i);
    if (b != CMSat::l_Undef) {
      assert(b == l_True || b == l_False);
      val = b == l_True;
      banLits.push_back(getLit(cnfVar, !val));
    }
    cmsSolver.add_clause(banLits);
    model[cnfVar] = val;
  }
  return model;
}

Lit SatSolver::getLit(Int cnfVar, bool val) {
  assert(cnfVar > 0);
  return Lit(cnfVar - 1, !val);
}

SatSolver::SatSolver(const Cnf& cnf) {
  cmsSolver.new_vars(cnf.declaredVarCount);
  for (const Clause& clause : cnf.clauses) {
    if (clause.xorFlag) {
      vector<unsigned> vars;
      bool rhs = true;
      for (Int literal : clause) {
        vars.push_back(abs(literal) - 1);
        if (literal < 0) {
          rhs = !rhs;
        }
      }
      cmsSolver.add_xor_clause(vars, rhs);
    }
    else {
      vector<Lit> lits;
      for (Int literal : clause) {
        lits.push_back(getLit(abs(literal), literal > 0));
      }
      cmsSolver.add_clause(lits);
    }
  }
}

/* class Dd ================================================================= */

size_t Dd::maxDdLeafCount;
size_t Dd::maxDdNodeCount;

size_t Dd::prunedDdCount;
Float Dd::pruningDuration;

bool Dd::dynOrderEnabled = false;

bool Dd::enableDynamicOrdering(const Cudd* mgr) {
  assert(ddPackage == CUDD_PACKAGE);
  mgr->AutodynEnable();
  Dd::dynOrderEnabled = true;
  mgr->AddHook(postReorderHook, CUDD_POST_REORDERING_HOOK);
  mgr->AddHook(preReorderHook, CUDD_PRE_REORDERING_HOOK);
  return (Dd::dynOrderEnabled);
}

bool Dd::disableDynamicOrdering(const Cudd* mgr) {
  assert(ddPackage == CUDD_PACKAGE);
  mgr->AutodynDisable();
  Dd::dynOrderEnabled = false;
  return (Dd::dynOrderEnabled);
}

int Dd::postReorderHook(DdManager *dd, const char *str, void *data){
    //TODO: 
    //read new var order from manager using cudd_readperm and cudd_readinvperm
    //    https://add-lib.scce.info/assets/doxygen-cudd-documentation/cuddAPI_8c.html#aacfa59899b792c9f47a612ceba42c976
    //    https://add-lib.scce.info/assets/doxygen-cudd-documentation/cuddAPI_8c.html#ae16ce73ed2e5afcd0dd1c1db43884d2a
    //    say int perm[] holds the permutation of vars from 1..n read using perm[i] = Cudd_ReadPerm(dd, i) 
    //    i.e. perm[i] = j means that the position of ith variable in the new order is j
    //
    // uint64_t nVars = JoinNode::cnf.apparentVars.size();
    // for (uint64_t i=0; i<nVars; i++){
    //   Cudd_ReadPerm(dd,i);
    // }     
    
    //make sure cnfVartoDDVar and ddVarToCNFVar maps are public static members of Executor class
    // update those maps using new var order.
    //    for i 1...n
    //        set cnfVarToDDVarMap[i] = perm[cnfVarToDDVarMap[i]]
    //        set ddVarToCnFVarmap[perm[cnfVarToDDVarMap[i]]] = i
              //where perm is as calculated in previous step    
    // use Cudd_AddHook or mgr->AddHook to add this function as a hook. (added in enableDynamicOrdering)
    //    https://add-lib.scce.info/assets/doxygen-cudd-documentation/cuddAPI_8c.html#a6884f064de544463f006f9104e4afa74
    //optionally additionally make Cudd* mgr a static member of Dd class
    
    unsigned long initialTime = (unsigned long) (ptruint) data;
    int retval;
    unsigned long finalTime = util_cpu_time();
    double totalTimeSec = (double)(finalTime - initialTime) / 1000.0;

    retval = fprintf(dd->out,"%ld nodes in %g sec\n", strcmp(str, "BDD") == 0 ? Cudd_ReadNodeCount(dd) : Cudd_zddReadNodeCount(dd), totalTimeSec);
    if (retval == EOF) return(0);
    retval = fflush(dd->out);
    if (retval == EOF) return(0);
    return(1);

}

int Dd::preReorderHook(DdManager *dd, const char *str, void *data){
    Cudd_ReorderingType method = (Cudd_ReorderingType) (ptruint) data;
    int retval;

    retval = fprintf(dd->out,"%s reordering with ", str);
    if (retval == EOF) return(0);
    switch (method) {
    case CUDD_REORDER_SIFT_CONVERGE:
    case CUDD_REORDER_SYMM_SIFT_CONV:
    case CUDD_REORDER_GROUP_SIFT_CONV:
    case CUDD_REORDER_WINDOW2_CONV:
    case CUDD_REORDER_WINDOW3_CONV:
    case CUDD_REORDER_WINDOW4_CONV:
    case CUDD_REORDER_LINEAR_CONVERGE:
        retval = fprintf(dd->out,"converging ");
        if (retval == EOF) return(0);
        break;
    default:
        break;
    }
    switch (method) {
    case CUDD_REORDER_RANDOM:
    case CUDD_REORDER_RANDOM_PIVOT:
        retval = fprintf(dd->out,"random");
        break;
    case CUDD_REORDER_SIFT:
    case CUDD_REORDER_SIFT_CONVERGE:
        retval = fprintf(dd->out,"sifting");
        break;
    case CUDD_REORDER_SYMM_SIFT:
    case CUDD_REORDER_SYMM_SIFT_CONV:
        retval = fprintf(dd->out,"symmetric sifting");
        break;
    case CUDD_REORDER_LAZY_SIFT:
        retval = fprintf(dd->out,"lazy sifting");
        break;
    case CUDD_REORDER_GROUP_SIFT:
    case CUDD_REORDER_GROUP_SIFT_CONV:
        retval = fprintf(dd->out,"group sifting");
        break;
    case CUDD_REORDER_WINDOW2:
    case CUDD_REORDER_WINDOW3:
    case CUDD_REORDER_WINDOW4:
    case CUDD_REORDER_WINDOW2_CONV:
    case CUDD_REORDER_WINDOW3_CONV:
    case CUDD_REORDER_WINDOW4_CONV:
        retval = fprintf(dd->out,"window");
        break;
    case CUDD_REORDER_ANNEALING:
        retval = fprintf(dd->out,"annealing");
        break;
    case CUDD_REORDER_GENETIC:
        retval = fprintf(dd->out,"genetic");
        break;
    case CUDD_REORDER_LINEAR:
    case CUDD_REORDER_LINEAR_CONVERGE:
        retval = fprintf(dd->out,"linear sifting");
        break;
    case CUDD_REORDER_EXACT:
        retval = fprintf(dd->out,"exact");
        break;
    default:
        return(0);
    }
    if (retval == EOF) return(0);

    retval = fprintf(dd->out,": from %ld to ... ", strcmp(str, "BDD") == 0 ? Cudd_ReadNodeCount(dd) : Cudd_zddReadNodeCount(dd));
    if (retval == EOF) return(0);
    fflush(dd->out);
    return(1);

} /* end of Cudd_StdPreReordHook */


size_t Dd::getLeafCount() const {
  if (ddPackage == CUDD_PACKAGE) {
    return cuadd.CountLeaves();
  }
  MTBDD d = mtbdd.GetMTBDD();
  return mtbdd_leafcount(d);
}

size_t Dd::getNodeCount() const {
  if (ddPackage == CUDD_PACKAGE) {
    return cuadd.nodeCount();
  }
  return mtbdd.NodeCount();
}

Dd::Dd(const ADD& cuadd) {
  assert(ddPackage == CUDD_PACKAGE);
  this->cuadd = cuadd;
}

Dd::Dd(const Mtbdd& mtbdd) {
  assert(ddPackage == SYLVAN_PACKAGE);
  this->mtbdd = mtbdd;
}

Dd::Dd(const Dd& dd) {
  if (ddPackage == CUDD_PACKAGE) {
    // *this = Dd(dd.cuadd);
    this->cuadd = dd.cuadd;
    this->cubdd = dd.cubdd;
  }
  else {
    // *this = Dd(dd.mtbdd);
    this->mtbdd = dd.mtbdd;
    this->sybdd = dd.sybdd;
  }

  // maxDdLeafCount = max(maxDdLeafCount, getLeafCount());
  // maxDdNodeCount = max(maxDdNodeCount, getNodeCount());
}

Number Dd::extractConst() const {
  if (ddPackage == CUDD_PACKAGE) {
    ADD minTerminal = cuadd.FindMin();
    assert(minTerminal == cuadd.FindMax());
    return Number(cuddV(minTerminal.getNode()));
  }
  assert(mtbdd.isLeaf());
  if (multiplePrecision) {
    uint64_t val = mtbdd_getvalue(mtbdd.GetMTBDD());
    return Number(mpq_class(reinterpret_cast<mpq_ptr>(val)));
  }
  return Number(mtbdd_getdouble(mtbdd.GetMTBDD()));
}

Dd Dd::getConstDd(const Number& n, const Cudd* mgr) {
  if (ddPackage == CUDD_PACKAGE) {
    return logCounting ? Dd(mgr->constant(n.getLog10())) : Dd(mgr->constant(n.fraction));
  }
  if (multiplePrecision) {
    mpq_t q; // C interface
    mpq_init(q);
    mpq_set(q, n.quotient.get_mpq_t());
    Dd dd(Mtbdd(mtbdd_gmp(q)));
    mpq_clear(q);
    return dd;
  }
  return Dd(Mtbdd::doubleTerminal(n.fraction));
}

Dd Dd::getZeroDd(const Cudd* mgr) {
  return getConstDd(Number(), mgr);
}

Dd Dd::getOneDd(const Cudd* mgr) {
  return getConstDd(Number("1"), mgr);
}

Dd Dd::getVarDd(Int ddVar, bool val, const Cudd* mgr) {
  if (ddPackage == CUDD_PACKAGE) {
    if (logCounting) {
      return Dd(mgr->addLogVar(ddVar, val));
    }
    ADD d = mgr->addVar(ddVar);
    return val ? Dd(d) : Dd(d.Cmpl());
  }
  MTBDD d0 = getZeroDd(mgr).mtbdd.GetMTBDD();
  MTBDD d1= getOneDd(mgr).mtbdd.GetMTBDD();
  return val ? Dd(Mtbdd(sylvan::mtbdd_makenode(ddVar, d0, d1))) : Dd(Mtbdd(sylvan::mtbdd_makenode(ddVar, d1, d0)));
}

Dd::Dd(const BDD& cubdd){
  assert(ddPackage == CUDD_PACKAGE);
  this->cubdd = cubdd;
}

Dd::Dd(const Bdd& sybdd) {
  assert(ddPackage == SYLVAN_PACKAGE);
  this->sybdd = sybdd;
}

Dd Dd::getZeroBdd(const Cudd* mgr) {
  if (ddPackage == CUDD_PACKAGE){
    return mgr->bddZero();
  } else{
    return Bdd::bddZero();
  }
}

Dd Dd::getOneBdd(const Cudd* mgr) {
  if (ddPackage == CUDD_PACKAGE){
    return mgr->bddOne();
  } else{
    return Bdd::bddOne();
  }
}

Dd Dd::getVarBdd(Int ddVar, bool val, const Cudd* mgr) {
  if (ddPackage == CUDD_PACKAGE) {
    BDD d = mgr->bddVar(ddVar);
    return val ? Dd(d) : Dd(!d);
  } else {
    return val? Bdd::bddVar(ddVar) : !Bdd::bddVar(ddVar); 
  }
}

Dd Dd::getBddAnd(const Dd& dd) const {
  if (ddPackage == CUDD_PACKAGE) {
    return Dd(cubdd * dd.cubdd);
  } else{
    return Dd(sybdd * dd.sybdd);
  }
}

Dd Dd::getBddExists(vector<Int> ddVars, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr) const {
  Dd cube = getOneBdd(mgr);
  for (auto& var : ddVars){
    cube = cube.getBddAnd(getVarBdd(var,true,mgr));
  }
  if (ddPackage == CUDD_PACKAGE){
    return cubdd.ExistAbstract(cube.cubdd);
  } else{
    return sybdd.ExistAbstract(sylvan::BddSet(cube.sybdd));
  }
}

Dd Dd::getBddOr(const Dd& dd) const {
  if(ddPackage == CUDD_PACKAGE){
    return Dd(cubdd.Or(dd.cubdd));
  } else{
    return Dd(sybdd.Or(dd.sybdd));
  }
}

bool Dd::isTrue() const{
  if(ddPackage == CUDD_PACKAGE){
     return cubdd.IsOne();
  } else{
    return sybdd.isOne();
  }
}

Set<Int> Dd::getBddSupport() const{
  Set<Int> support;
  if (ddPackage == CUDD_PACKAGE) {
    for (Int ddVar : cubdd.SupportIndices()) {
      support.insert(ddVar);
    }
   
  }
  else {
    Mtbdd cube = sybdd.Support(); // conjunction of all vars appearing in mtbdd
    while (!cube.isOne()) {
      support.insert(cube.TopVar());
      cube = cube.Then();
    }
  }
  return support;
}

Dd Dd::getFilteredBdd(const Dd other, const Cudd* mgr){
  Set<Int> otherSupport = other.getBddSupport();
  for (const auto& elem : getBddSupport()) {
    otherSupport.erase(elem);
  }

  Dd cube = getOneBdd(mgr);
  for (auto& var : otherSupport){
    cube = cube.getBddAnd(getVarBdd(var,true,mgr));
  }
  if (ddPackage == CUDD_PACKAGE){
    return Dd(cubdd.AndAbstract(other.cubdd, cube.cubdd));
  } else{
    return Dd(sybdd.AndAbstract(other.sybdd,cube.sybdd));
  }
}

const Cudd* Dd::newMgr(Float mem, Int threadIndex) {
  assert(ddPackage == CUDD_PACKAGE);
  Cudd* mgr = new Cudd(
    0, // init num of BDD vars
    0, // init num of ZDD vars
    CUDD_UNIQUE_SLOTS, // init num of unique-table slots; cudd.h: #define CUDD_UNIQUE_SLOTS 256
    CUDD_CACHE_SLOTS, // init num of cache-table slots; cudd.h: #define CUDD_CACHE_SLOTS 262144
    mem * MEGA // maxMemory
  );
  mgr->getManager()->threadIndex = threadIndex;
  mgr->getManager()->peakMemIncSensitivity = memSensitivity * MEGA; // makes CUDD print "c cuddMegabytes_{threadIndex + 1} {memused / 1e6}"
  if (verboseSolving >= 4 && threadIndex == 0) {
    printRow("hardMaxMemMegabytes", mgr->ReadMaxMemory() / MEGA); // for unique table and cache table combined (unlimited by default)
    printRow("softMaxMemMegabytes", mgr->getManager()->maxmem / MEGA); // cuddInt.c: maxmem = maxMemory / 10 * 9
    printRow("hardMaxCacheMegabytes", mgr->ReadMaxCacheHard() * sizeof(DdCache) / MEGA); // cuddInt.h: #define DD_MAX_CACHE_FRACTION 3
    writeInfoFile(mgr, "cudd.txt");
  }
  return mgr;
}

bool Dd::operator!=(const Dd& rightDd) const {
  if (ddPackage == CUDD_PACKAGE) {
    return cuadd != rightDd.cuadd;
  }
  return mtbdd != rightDd.mtbdd;
}

bool Dd::operator<(const Dd& rightDd) const {
  if (joinPriority == SMALLEST_PAIR) { // top = rightmost = smallest
    return getNodeCount() > rightDd.getNodeCount();
  }
  return getNodeCount() < rightDd.getNodeCount();
}

Dd Dd::getComposition(Int ddVar, bool val, const Cudd* mgr) const {
  if (ddPackage == CUDD_PACKAGE) {
    if (util::isFound(ddVar, cuadd.SupportIndices())) {
      return Dd(cuadd.Compose(val ? mgr->addOne() : mgr->addZero(), ddVar));
    }
    return *this;
  }
  sylvan::MtbddMap m;
  m.put(ddVar, val ? Mtbdd::mtbddOne() : Mtbdd::mtbddZero());
  return Dd(mtbdd.Compose(m));
}

Dd Dd::getProduct(const Dd& dd) const {
  if (ddPackage == CUDD_PACKAGE) {
    return logCounting ? Dd(cuadd + dd.cuadd) : Dd(cuadd * dd.cuadd);
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_times(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd * dd.mtbdd);
}

Dd Dd::getSum(const Dd& dd) const {
  if (ddPackage == CUDD_PACKAGE) {
    return logCounting ? Dd(cuadd.LogSumExp(dd.cuadd)) : Dd(cuadd + dd.cuadd);
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_plus(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd + dd.mtbdd);
}

Dd Dd::getMax(const Dd& dd) const {
  if (ddPackage == CUDD_PACKAGE) {
    return Dd(cuadd.Maximum(dd.cuadd));
  }
  if (multiplePrecision) {
    LACE_ME;
    return Dd(Mtbdd(gmp_max(mtbdd.GetMTBDD(), dd.mtbdd.GetMTBDD())));
  }
  return Dd(mtbdd.Max(dd.mtbdd));
}

Dd Dd::getXor(const Dd& dd) const {
  assert(ddPackage == CUDD_PACKAGE);
  return logCounting ? Dd(cuadd.LogXor(dd.cuadd)) : Dd(cuadd.Xor(dd.cuadd));
}

Set<Int> Dd::getSupport() const {
  Set<Int> support;
  if (ddPackage == CUDD_PACKAGE) {
    for (Int ddVar : cuadd.SupportIndices()) {
      support.insert(ddVar);
    }
  }
  else {
    Mtbdd cube = mtbdd.Support(); // conjunction of all vars appearing in mtbdd
    while (!cube.isOne()) {
      support.insert(cube.TopVar());
      cube = cube.Then();
    }
  }
  return support;
}

Dd Dd::getBoolDiff(const Dd& rightDd) const {
  assert(ddPackage == CUDD_PACKAGE);
  return Dd((cuadd - rightDd.cuadd).BddThreshold(0).Add());
}

bool Dd::evalAssignment(vector<int>& ddVarAssignment) const {
  assert(ddPackage == CUDD_PACKAGE);
  Number n = Dd(cuadd.Eval(&ddVarAssignment.front())).extractConst();
  return n == Number(1);
}

Dd Dd::getAbstraction(Int ddVar, const vector<Int>& ddVarToCnfVarMap, const Map<Int, Number>& literalWeights, const Assignment& assignment, bool additiveFlag, vector<pair<Int, Dd>>& maximizationStack, const Cudd* mgr) const {
  Int cnfVar = ddVarToCnfVarMap.at(ddVar);
  Dd positiveWeight = getConstDd(literalWeights.at(cnfVar), mgr);
  Dd negativeWeight = getConstDd(literalWeights.at(-cnfVar), mgr);

  auto it = assignment.find(cnfVar);
  if (it != assignment.end()) {
    Dd weight = it->second ? positiveWeight : negativeWeight;
    return getProduct(weight);
  }

  Dd highTerm = getComposition(ddVar, true, mgr).getProduct(positiveWeight);
  Dd lowTerm = getComposition(ddVar, false, mgr).getProduct(negativeWeight);

  if (maximizerFormat && !additiveFlag) {
    Dd dsgn = highTerm.getBoolDiff(lowTerm); // derivative sign
    maximizationStack.push_back({ddVar, dsgn});
    if (substitutionMaximization) {
      return Dd(cuadd.Compose(dsgn.cuadd, ddVar));
    }
  }

  return additiveFlag ? highTerm.getSum(lowTerm) : highTerm.getMax(lowTerm);
}

Dd Dd::getPrunedDd(Float lowerBound, const Cudd* mgr) const {
  assert(logCounting);

  TimePoint pruningStartPoint = util::getTimePoint();

  ADD bound = mgr->constant(lowerBound);
  ADD prunedDd = cuadd.LogThreshold(bound);

  pruningDuration += util::getDuration(pruningStartPoint);

  if (prunedDd != cuadd) {
    prunedDdCount++;
  }

  return Dd(prunedDd);
}

void Dd::writeDotFile(const Cudd* mgr, const string& dotFileDir) const {
  string filePath = dotFileDir + "dd" + to_string(dotFileIndex++) + ".dot";
  FILE* file = fopen(filePath.c_str(), "wb"); // writes to binary file

  if (ddPackage == CUDD_PACKAGE) { // davidkebo.com/cudd#cudd6
    DdNode** ddNodeArray = static_cast<DdNode**>(malloc(sizeof(DdNode*)));
    ddNodeArray[0] = cuadd.getNode();
    Cudd_DumpDot(mgr->getManager(), 1, ddNodeArray, NULL, NULL, file);
    free(ddNodeArray);
  }
  else {
    mtbdd_fprintdot_nc(file, mtbdd.GetMTBDD());
  }

  fclose(file);
  cout << "c wrote decision diagram to file " << filePath << "\n";
}

void Dd::writeInfoFile(const Cudd* mgr, const string& filePath) {
  assert(ddPackage == CUDD_PACKAGE);
  FILE* file = fopen(filePath.c_str(), "w");
  Cudd_PrintInfo(mgr->getManager(), file);
  fclose(file);
  cout << "c wrote CUDD info to file " << filePath << "\n";
}

/* class SatFilter =========================================================== */

Dd SatFilter::getClauseBdd(const Map<Int, Int>& cnfVarToDdVarMap, const Clause& clause, const Cudd* mgr){
  Dd clauseDd = Dd::getZeroBdd(mgr);
  for (Int literal : clause) {
    bool val = literal > 0;
    Int cnfVar = abs(literal);
    Int ddVar = cnfVarToDdVarMap.at(cnfVar);
    Dd literalDd = Dd::getVarBdd(ddVar, val, mgr);
    clauseDd = clauseDd.getBddOr(literalDd);
  }
  return clauseDd;
}

Dd SatFilter::solveSubtree(const JoinNode* joinNode, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr) {
  if (joinNode->isTerminal()) {
    TimePoint terminalStartPoint = util::getTimePoint();
    // cout << "c 1\n";
    Dd d = getClauseBdd(cnfVarToDdVarMap, JoinNode::cnf.clauses.at(joinNode->nodeIndex), mgr);
    return d;
  }

  vector<Dd> childDdList;
  for (JoinNode* child : joinNode->children) {
    // cout << "c before recurse\n";
    childDdList.push_back(solveSubtree(child, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr));
    // Dd d = solveSubtree(child, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr);
    // cout << "c mid recurse\n";
    // childDdList.push_back(d);
    // cout << "c after recurse\n";
  }

  TimePoint nonterminalStartPoint = util::getTimePoint();
  // cout << "c 3\n";
  Dd dd = Dd::getOneBdd(mgr);
  // cout << "c 4\n";
  // if (joinPriority == ARBITRARY_PAIR) { // arbitrarily multiplies child decision diagrams
    for (Dd childDd : childDdList) {
      // cout << "c 5\n";
      dd = dd.getBddAnd(childDd);
      // cout << "c 6\n";
    }
  // }
  
  if (joinNode->projectionVars.size()>0){
    Dd* d = new Dd(dd); // to make sure it does not go out of scope since we are storing a pointer
    ((JoinNode*) joinNode)->dd = (void*) d; //cast away const-ness of joinNode to modify dd. Should be fine since object itself is not const
    //joinNode->dd = (void*)&dd;
  }
    

  vector<Int> ddVars;
  for (Int cnfVar : joinNode->projectionVars) {
    ddVars.push_back(cnfVarToDdVarMap.at(cnfVar));
  }
  // cout << "c 7\n";
  dd = dd.getBddExists(ddVars, ddVarToCnfVarMap, mgr);
  // cout << "c 8\n";
  return dd;
}

bool SatFilter::solveCnf(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr) {
  if (ddPackage == SYLVAN_PACKAGE) {
    return solveSubtree(
      static_cast<const JoinNode*>(joinRoot),
      cnfVarToDdVarMap,
      ddVarToCnfVarMap
    ).isTrue();
  } else{
    cout << "c Starting solvesubtree in SatFilter ...\n";
    return solveSubtree(static_cast<const JoinNode*>(joinRoot), cnfVarToDdVarMap, ddVarToCnfVarMap, mgr).isTrue();
  }
}

bool SatFilter::filterBdds(const JoinNode* joinNode, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, Dd parentBdd, const Cudd* mgr){
  Dd* d = (Dd*)joinNode->dd;
  // if(d == 0){
  //   return false;
  // }
  if (joinNode->projectionVars.size()==0){
    assert(d==0);
    d = &parentBdd;   
  } else{
    Dd temp = d->getFilteredBdd(parentBdd, mgr);
    delete d;
    d = new Dd(temp); // to make sure it does not go out of scope since we are storing a pointer
    ((JoinNode*) joinNode)->dd = (void*) d;
  }
  for (JoinNode* child : joinNode->children) {
    filterBdds(child, cnfVarToDdVarMap, ddVarToCnfVarMap, *d, mgr);
  }
  return true;
}

SatFilter::SatFilter(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic) {
  cout << "\n";
  cout << "c constructing SatFilter. Note: leaf and node counting functions may not return right answer (see Dd copy constructor comment)..\n";

  TimePoint ddVarOrderStartPoint = util::getTimePoint();
  vector<Int> ddVarToCnfVarMap = joinRoot->getVarOrder(ddVarOrderHeuristic); // e.g. [42, 13], i.e. ddVarOrder
  if (verboseSolving >= 1) {
    printRow("diagramVarSeconds", util::getDuration(ddVarOrderStartPoint));
  }

  Map<Int, Int> cnfVarToDdVarMap; // e.g. {42: 0, 13: 1}
  for (Int ddVar = 0; ddVar < ddVarToCnfVarMap.size(); ddVar++) {
    Int cnfVar = ddVarToCnfVarMap.at(ddVar);
    cnfVarToDdVarMap[cnfVar] = ddVar;
  }

  const Cudd* mgr = Dd::newMgr(maxMem, 0);
  if(dynVarOrdering == 1){
    Dd::enableDynamicOrdering(mgr);
  }

  bool solution = solveCnf(joinRoot, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr);
  if (!solution){
    cout << "c Formula is UNSAT. Exiting..\n";
    exit(1);
  }
  cout << "c Done constructing SatFilter. Applying SatFilter...\n";
  filterBdds(joinRoot,cnfVarToDdVarMap,ddVarToCnfVarMap,Dd::getOneBdd(mgr),mgr);
  cout << "c Done Applying SatFilter!\n";
  //printVarDurations();
  //printVarDdSizes();

  // printRow("maxDiagramLeaves", Dd::maxDdLeafCount);
  // printRow("maxDiagramNodes", Dd::maxDdNodeCount);

  // if (verboseSolving >= 1) {
  //   printRow("formula is SAT? 1/0: ", solution);
  // }

}

/* class Executor =========================================================== */

vector<pair<Int, Dd>> Executor::maximizationStack;

Map<Int, Float> Executor::varDurations;
Map<Int, size_t> Executor::varDdSizes;

void Executor::updateVarDurations(const JoinNode* joinNode, TimePoint startPoint) {
  if (verboseProfiling >= 1) {
    Float duration = util::getDuration(startPoint);
    if (duration > 0) {
      if (verboseProfiling >= 2) {
        printRow("solvingSeconds_joinNode" + to_string(joinNode->nodeIndex + 1), duration);
      }

      for (Int var : joinNode->preProjectionVars) {
        if (varDurations.contains(var)) {
          varDurations[var] += duration;
        }
        else {
          varDurations[var] = duration;
        }
      }
    }
  }
}

void Executor::updateVarDdSizes(const JoinNode* joinNode, const Dd& dd) {
  if (verboseProfiling >= 1) {
    size_t ddSize = dd.getNodeCount();

    if (verboseProfiling >= 2) {
      printRow("diagramNodes_joinNode" + to_string(joinNode->nodeIndex + 1), ddSize);
    }

    for (Int var : joinNode->preProjectionVars) {
      if (varDdSizes.contains(var)) {
        varDdSizes[var] = max(varDdSizes[var], ddSize);
      }
      else {
        varDdSizes[var] = ddSize;
      }
    }
  }
}

void Executor::printVarDurations() {
  multimap<Float, Int, greater<Float>> timedVars = util::flipMap(varDurations); // duration |-> var
  for (pair<Float, Int> timedVar : timedVars) {
    printRow("totalSeconds_var" + to_string(timedVar.second), timedVar.first);
  }
}

void Executor::printVarDdSizes() {
  multimap<size_t, Int, greater<size_t>> sizedVars = util::flipMap(varDdSizes); // DD size |-> var
  for (pair<size_t, Int> sizedVar : sizedVars) {
    printRow("maxDiagramNodes_var" + to_string(sizedVar.second), sizedVar.first);
  }
}

Dd Executor::getClauseDd(const Map<Int, Int>& cnfVarToDdVarMap, const Clause& clause, const Cudd* mgr, const Assignment& assignment) {
  Dd clauseDd = Dd::getZeroDd(mgr);
  for (Int literal : clause) {
    bool val = literal > 0;
    Int cnfVar = abs(literal);
    auto it = assignment.find(cnfVar);
    if (it != assignment.end()) { // literal has assigned value
      if (it->second == val) {
        if (clause.xorFlag) { // flips polarity
          clauseDd = clauseDd.getXor(Dd::getOneDd(mgr));
        }
        else { // returns satisfied disjunctive clause
          return Dd::getOneDd(mgr);
        }
      } // excludes unsatisfied literal from clause otherwise
    }
    else { // literal is unassigned
      Int ddVar = cnfVarToDdVarMap.at(cnfVar);
      Dd literalDd = Dd::getVarDd(ddVar, val, mgr);
      clauseDd = clause.xorFlag ? clauseDd.getXor(literalDd) : clauseDd.getMax(literalDd);
    }
  }
  return clauseDd;
}

Dd Executor::solveSubtree(const JoinNode* joinNode, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, const Cudd* mgr, const Assignment& assignment) {
  if (joinNode->isTerminal()) {
    TimePoint terminalStartPoint = util::getTimePoint();

    Dd d = getClauseDd(cnfVarToDdVarMap, JoinNode::cnf.clauses.at(joinNode->nodeIndex), mgr, assignment);

    updateVarDurations(joinNode, terminalStartPoint);
    updateVarDdSizes(joinNode, d);

    return d;
  }

  vector<Dd> childDdList;
  for (JoinNode* child : joinNode->children) {
    childDdList.push_back(solveSubtree(child, cnfVarToDdVarMap, ddVarToCnfVarMap, mgr, assignment));
  }

  TimePoint nonterminalStartPoint = util::getTimePoint();
  Dd dd = Dd::getOneDd(mgr);

  if (joinPriority == ARBITRARY_PAIR) { // arbitrarily multiplies child decision diagrams
    for (Dd childDd : childDdList) {
      dd = dd.getProduct(childDd);
    }
  }
  else { // Dd::operator< handles both biggest-first and smallest-first
    std::priority_queue<Dd> childDdQueue;
    for (Dd childDd : childDdList) {
      childDdQueue.push(childDd);
    }
    assert(!childDdQueue.empty());
    while (childDdQueue.size() >= 2) {
      Dd dd1 = childDdQueue.top();
      childDdQueue.pop();
      Dd dd2 = childDdQueue.top();
      childDdQueue.pop();
      Dd dd3 = dd1.getProduct(dd2);
      childDdQueue.push(dd3);
    }
    dd = childDdQueue.top();
  }

  for (Int cnfVar : joinNode->projectionVars) {
    Int ddVar = cnfVarToDdVarMap.at(cnfVar);

    bool additiveFlag = JoinNode::cnf.outerVars.contains(cnfVar);
    if (existRandom) {
      additiveFlag = !additiveFlag;
    }

    dd = dd.getAbstraction(ddVar, ddVarToCnfVarMap, JoinNode::cnf.literalWeights, assignment, additiveFlag, maximizationStack, mgr);

    if (logBound > -INF) {
      if (JoinNode::cnf.literalWeights.at(cnfVar) != Number(1) || JoinNode::cnf.literalWeights.at(-cnfVar) != Number(1)) {
        Dd prunedDd = dd.getPrunedDd(logBound, mgr);
        if (prunedDd != dd) {
          if (verboseSolving >= 3) {
            cout << "c writing pre-pruning decision diagram...\n";
            dd.writeDotFile(mgr);

            cout << "c writing post-pruning decision diagram...\n";
            prunedDd.writeDotFile(mgr);
          }
          dd = prunedDd;
        }
      }
    }
  }

  updateVarDurations(joinNode, nonterminalStartPoint);
  updateVarDdSizes(joinNode, dd);

  return dd;
}

void Executor::solveThreadSlices(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, Float threadMem, Int threadIndex, const vector<vector<Assignment>>& threadAssignmentLists, Number& totalSolution, mutex& solutionMutex) {
  const vector<Assignment>& threadAssignments = threadAssignmentLists.at(threadIndex);
  for (Int threadAssignmentIndex = 0; threadAssignmentIndex < threadAssignments.size(); threadAssignmentIndex++) {
    TimePoint sliceStartPoint = util::getTimePoint();
    
    const Cudd* mgr = Dd::newMgr(threadMem, threadIndex);
    if(dynVarOrdering == 1){
      Dd::enableDynamicOrdering(mgr);
    }
    Number partialSolution = solveSubtree(static_cast<const JoinNode*>(joinRoot), cnfVarToDdVarMap, ddVarToCnfVarMap, mgr, threadAssignments.at(threadAssignmentIndex)).extractConst();

    const std::lock_guard<mutex> g(solutionMutex);

    if (verboseSolving >= 1) {
      cout << "c thread " << right << setw(4) << threadIndex + 1 << "/" << threadAssignmentLists.size();
      cout << " | assignment " << setw(4) << threadAssignmentIndex + 1 << "/" << threadAssignments.size();

      cout << ": { ";
      threadAssignments.at(threadAssignmentIndex).printAssignment();
      cout << " }\n";

      cout << "c thread " << right << setw(4) << threadIndex + 1 << "/" << threadAssignmentLists.size();
      cout << " | assignment " << setw(4) << threadAssignmentIndex + 1 << "/" << threadAssignments.size();
      cout << " | seconds " << std::fixed << setw(10) << util::getDuration(sliceStartPoint);
      cout << " | solution " << setw(15) << partialSolution << "\n";
    }

    if (existRandom) {
      totalSolution = max(totalSolution, partialSolution);
    }
    else {
      totalSolution = logCounting ? Number(totalSolution.getLogSumExp(partialSolution)) : totalSolution + partialSolution;
    }
  }
}

vector<vector<Assignment>> Executor::getThreadAssignmentLists(const JoinNonterminal* joinRoot, Int sliceVarOrderHeuristic) {
  size_t sliceVarCount = ceill(log2l(threadCount * threadSliceCount));
  sliceVarCount = min(sliceVarCount, JoinNode::cnf.outerVars.size());

  Int remainingSliceCount = exp2l(sliceVarCount);
  Int remainingThreadCount = threadCount;
  vector<Int> threadSliceCounts;
  while (remainingThreadCount > 0) {
    Int sliceCount = ceill(static_cast<Float>(remainingSliceCount) / remainingThreadCount);
    threadSliceCounts.push_back(sliceCount);
    remainingSliceCount -= sliceCount;
    remainingThreadCount--;
  }
  assert(remainingSliceCount == 0);

  if (verboseSolving >= 1) {
    cout << "c thread slice counts: { ";
    for (Int sliceCount : threadSliceCounts) {
      cout << sliceCount << " ";
    }
    cout << "}\n";
  }

  vector<Assignment> assignments = joinRoot->getOuterAssignments(sliceVarOrderHeuristic, sliceVarCount);
  vector<vector<Assignment>> threadAssignmentLists;
  vector<Assignment> threadAssignmentList;
  for (Int assignmentIndex = 0, threadListIndex = 0; assignmentIndex < assignments.size() && threadListIndex < threadSliceCounts.size(); assignmentIndex++) {
    threadAssignmentList.push_back(assignments.at(assignmentIndex));
    if (threadAssignmentList.size() == threadSliceCounts.at(threadListIndex)) {
      threadAssignmentLists.push_back(threadAssignmentList);
      threadAssignmentList.clear();
      threadListIndex++;
    }
  }

  if (verboseSolving >= 2) {
    for (Int threadIndex = 0; threadIndex < threadAssignmentLists.size(); threadIndex++) {
      const vector<Assignment>& threadAssignments = threadAssignmentLists.at(threadIndex);
      cout << "c assignments in thread " << right << setw(4) << threadIndex + 1 << ":";
      for (const Assignment& assignment : threadAssignments) {
        cout << " { ";
        assignment.printAssignment();
        cout << " }";
      }
      cout << "\n";
    }
  }

  return threadAssignmentLists;
}

Number Executor::solveCnf(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap, Int sliceVarOrderHeuristic) {
  if (ddPackage == SYLVAN_PACKAGE) {
    return solveSubtree(
      static_cast<const JoinNode*>(joinRoot),
      cnfVarToDdVarMap,
      ddVarToCnfVarMap
    ).extractConst();
  }

  vector<vector<Assignment>> threadAssignmentLists = getThreadAssignmentLists(joinRoot, sliceVarOrderHeuristic);
  printRow("sliceWidth", joinRoot->getWidth(threadAssignmentLists.front().front())); // any assignment would work
  Number totalSolution = logCounting ? Number(-INF) : Number();
  mutex solutionMutex;

  Float threadMem = maxMem / threadAssignmentLists.size();
  printRow("threadMaxMemMegabytes", threadMem);

  vector<thread> threads;

  Int threadIndex = 0;
  for (; threadIndex < threadAssignmentLists.size() - 1; threadIndex++) {
    threads.push_back(thread(
      solveThreadSlices,
      std::cref(joinRoot),
      std::cref(cnfVarToDdVarMap),
      std::cref(ddVarToCnfVarMap),
      threadMem,
      threadIndex,
      threadAssignmentLists,
      std::ref(totalSolution),
      std::ref(solutionMutex)
    ));
  }
  solveThreadSlices(
    joinRoot,
    cnfVarToDdVarMap,
    ddVarToCnfVarMap,
    threadMem,
    threadIndex,
    threadAssignmentLists,
    totalSolution,
    solutionMutex
  );
  for (thread& t : threads) {
    t.join();
  }

  return totalSolution;
}

void Executor::setLogBound(const JoinNonterminal* joinRoot, const Map<Int, Int>& cnfVarToDdVarMap, const vector<Int>& ddVarToCnfVarMap) {
  if (logBound > -INF) {} // LOG_BOUND_OPTION
  else if (!thresholdModel.empty()) { // THRESHOLD_MODEL_OPTION
    logBound = solveSubtree(
      joinRoot,
      cnfVarToDdVarMap,
      ddVarToCnfVarMap,
      Dd::newMgr(maxMem),
      Assignment(thresholdModel)
    ).extractConst().fraction;
    printRow("logBound", logBound);
  }
  else if (satSolverPruning) { // SAT_SOLVER_PRUNING
    SatSolver satSolver(joinRoot->cnf);
    satSolver.checkSat(true);
    Assignment model = satSolver.getModel();
    logBound = solveSubtree(
      joinRoot,
      cnfVarToDdVarMap,
      ddVarToCnfVarMap,
      Dd::newMgr(maxMem),
      model
    ).extractConst().fraction;
    printRow("logBound", logBound);
    cout << "c " << getShortModel(model, joinRoot->cnf.declaredVarCount) << "\n";
  }
}

Number Executor::adjustSolutionToHiddenVar(const Number &apparentSolution, Int cnfVar, bool additiveFlag) {
  if (JoinNode::cnf.apparentVars.contains(cnfVar)) {
    return apparentSolution;
  }

  const Number& positiveWeight = JoinNode::cnf.literalWeights.at(cnfVar);
  const Number& negativeWeight = JoinNode::cnf.literalWeights.at(-cnfVar);
  if (additiveFlag) {
    return logCounting ? (apparentSolution + (positiveWeight + negativeWeight).getLog10()) : (apparentSolution * (positiveWeight + negativeWeight));
  }
  else {
    return logCounting ? (apparentSolution + max(positiveWeight, negativeWeight).getLog10()) : (apparentSolution * max(positiveWeight, negativeWeight)); // weights are positive
  }
}

Number Executor::getAdjustedSolution(const Number &apparentSolution) {
  Number n = apparentSolution;

  for (Int var = 1; var <= JoinNode::cnf.declaredVarCount; var++) { // processes inner vars
    if (!JoinNode::cnf.outerVars.contains(var)) {
      n = adjustSolutionToHiddenVar(n, var, existRandom);
    }
  }

  for (Int var : JoinNode::cnf.outerVars) {
    n = adjustSolutionToHiddenVar(n, var, !existRandom);
  }

  return n;
}

void Executor::printSatRow(const Number& solution, bool unsatFlag, size_t keyWidth) {
  const string SAT_WORD = "SATISFIABLE";
  const string UNSAT_WORD = "UN" + SAT_WORD;

  string satisfiability = "UNKNOWN";

  if (unsatFlag) {
    satisfiability = UNSAT_WORD;
  }
  else if (satSolverPruning) {
    satisfiability = SAT_WORD; // otherwise, UnsatSolverException would have been thrown earlier
  }
  else if (logCounting) {
    if (solution == Number(-INF)) {
      if (!weightedCounting) {
        satisfiability = UNSAT_WORD;
      }
    }
    else {
      satisfiability = SAT_WORD;
    }
  }
  else {
    if (solution == Number()) {
      if (!weightedCounting || multiplePrecision) {
        satisfiability = UNSAT_WORD;
      }
    }
    else {
      satisfiability = SAT_WORD;
    }
  }

  printRow("s", satisfiability, keyWidth);
}

void Executor::printTypeRow(size_t keyWidth) {
  string type = "maximum";
  if (!existRandom) {
    type = "mc";
    if (weightedCounting) {
      type = "w" + type;
    }
    if (projectedCounting) {
      type = "p" + type;
    }
  }
  printRow("s type", type, keyWidth);
}

void Executor::printEstRow(const Number& solution, size_t keyWidth) {
  printRow("s log10-estimate", logCounting ? solution.fraction : solution.getLog10(), keyWidth);
}

void Executor::printArbRow(const Number& solution, bool frac, size_t keyWidth) {
  string key = "s exact arb ";

  if (weightedCounting) {
    if (frac) {
      printRow(key + "frac", solution, keyWidth);
    }
    else {
      printRow(key + "float", mpf_class(solution.quotient), keyWidth);
    }
  }
  else {
    printRow(key + "int", solution, keyWidth);
  }
}

void Executor::printDoubleRow(const Number& solution, size_t keyWidth) {
  Float f = solution.fraction;
  printRow("s exact double prec-sci", logCounting ? exp10l(f) : f, keyWidth);
}

Number Executor::printAdjustedSolutionRows(const Number& solution, bool unsatFlag, size_t keyWidth) {
  cout << DASH_LINE;
  Number adjustedSolution = getAdjustedSolution(solution);

  printSatRow(adjustedSolution, unsatFlag, keyWidth);
  printTypeRow(keyWidth);
  printEstRow(adjustedSolution, keyWidth);

  if (multiplePrecision) {
    printArbRow(adjustedSolution, false, keyWidth); // notation = weighted ? int : float
    if (weightedCounting) {
      printArbRow(adjustedSolution, true, keyWidth); // notation = frac
    }
  }
  else {
    printDoubleRow(adjustedSolution, keyWidth);
  }

  cout << DASH_LINE;
  return adjustedSolution;
}

string Executor::getShortModel(const Assignment& model, Int declaredVarCount) {
  string s;
  for (Int cnfVar = 1; cnfVar <= declaredVarCount; cnfVar++) {
    s += to_string(model.getValue(cnfVar));
  }
  return s;
}

string Executor::getLongModel(const Assignment& model, Int declaredVarCount) {
  string s;
  for (Int cnfVar = 1; cnfVar <= declaredVarCount; cnfVar++) {
    s += (model.getValue(cnfVar) ? " " : " -") + to_string(cnfVar);
  }
  return s;
}

void Executor::printShortMaximizer(const Assignment& maximizer, Int declaredVarCount) {
  cout << "v ";
  cout << getShortModel(maximizer, declaredVarCount);
  cout << "\n";
}

void Executor::printLongMaximizer(const Assignment& maximizer, Int declaredVarCount) {
  cout << "v";
  cout << getLongModel(maximizer, declaredVarCount);
  cout << "\n";
}

Assignment Executor::printMaximizerRows(const vector<Int>& ddVarToCnfVarMap, Int declaredVarCount) {
  vector<int> ddVarAssignment(ddVarToCnfVarMap.size(), -1); // uses init value -1 (neither 0 nor 1) to test assertion in function Cudd_Eval
  Assignment cnfVarAssignment;

  while (!maximizationStack.empty()) {
    pair<Int, Dd> ddVarAndDsgn = maximizationStack.back();
    Int ddVar = ddVarAndDsgn.first;
    Dd dsgn = ddVarAndDsgn.second;

    bool val = dsgn.evalAssignment(ddVarAssignment);
    ddVarAssignment[ddVar] = val;
    cnfVarAssignment.insert({ddVarToCnfVarMap.at(ddVar), val});

    maximizationStack.pop_back();
  }

  switch (maximizerFormat) {
    case NEITHER_FORMAT:
      break;
    case SHORT_FORMAT:
      printShortMaximizer(cnfVarAssignment, declaredVarCount);
      break;
    case LONG_FORMAT:
      printLongMaximizer(cnfVarAssignment, declaredVarCount);
      break;
    default:
      printShortMaximizer(cnfVarAssignment, declaredVarCount);
      printLongMaximizer(cnfVarAssignment, declaredVarCount);
  }

  return cnfVarAssignment;
}

Number Executor::verifyMaximizer(
  const JoinNonterminal* joinRoot,
  const Map<Int, Int>& cnfVarToDdVarMap,
  const vector<Int>& ddVarToCnfVarMap,
  const Assignment& maximizer
) {
  Dd dd = solveSubtree(
    joinRoot,
    cnfVarToDdVarMap,
    ddVarToCnfVarMap,
    Dd::newMgr(maxMem),
    maximizer
  );
  Number solution = dd.extractConst();
  return getAdjustedSolution(solution);
}

Executor::Executor(const JoinNonterminal* joinRoot, Int ddVarOrderHeuristic, Int sliceVarOrderHeuristic) {
  cout << "\n";
  cout << "c computing output...\n";

  TimePoint ddVarOrderStartPoint = util::getTimePoint();
  vector<Int> ddVarToCnfVarMap = joinRoot->getVarOrder(ddVarOrderHeuristic); // e.g. [42, 13], i.e. ddVarOrder
  if (verboseSolving >= 1) {
    printRow("diagramVarSeconds", util::getDuration(ddVarOrderStartPoint));
  }

  Map<Int, Int> cnfVarToDdVarMap; // e.g. {42: 0, 13: 1}
  for (Int ddVar = 0; ddVar < ddVarToCnfVarMap.size(); ddVar++) {
    Int cnfVar = ddVarToCnfVarMap.at(ddVar);
    cnfVarToDdVarMap[cnfVar] = ddVar;
  }

  setLogBound(joinRoot, cnfVarToDdVarMap, ddVarToCnfVarMap);

  Number solution = solveCnf(joinRoot, cnfVarToDdVarMap, ddVarToCnfVarMap, sliceVarOrderHeuristic);

  printVarDurations();
  printVarDdSizes();

  if (logBound > -INF) {
    printRow("prunedDiagrams", Dd::prunedDdCount);
    printRow("pruningSeconds", Dd::pruningDuration);
  }

  // printRow("maxDiagramLeaves", Dd::maxDdLeafCount);
  // printRow("maxDiagramNodes", Dd::maxDdNodeCount);

  if (verboseSolving >= 1) {
    printRow("apparentSolution", solution);
  }

  solution = printAdjustedSolutionRows(solution);

  if (maximizerFormat) {
    Assignment maximizer = printMaximizerRows(ddVarToCnfVarMap, joinRoot->cnf.declaredVarCount);
    if (maximizerVerification) {
      TimePoint maximizerVerificationStartPoint = util::getTimePoint();
      Number maximizerSolution = verifyMaximizer(
        joinRoot,
        cnfVarToDdVarMap,
        ddVarToCnfVarMap,
        maximizer
      );
      printRow("adjustedSolution", solution);
      printRow("maximizerSolution", maximizerSolution);
      printRow("solutionMatch", (solution - maximizerSolution).getAbsolute() < Number("1/1000000")); // 1e-6 is tolerance in ProCount paper
      if (verboseSolving >= 1) {
        printRow("maximizerVerificationSeconds", util::getDuration(maximizerVerificationStartPoint));
      }
    }
  }
}

/* class OptionRequirement ================================================== */

OptionRequirement::OptionRequirement(const string& name, const string& value, const string& comparator) {
  this->name = name;
  this->value = value;
  this->comparator = comparator;
}

string OptionRequirement::getRequirement() const {
  return name + "_arg " + comparator + " " + value;
}

/* class OptionDict ========================================================= */

string OptionDict::requireOptions(const vector<OptionRequirement>& requirements) {
  string s = " [needs ";
  for (auto it = requirements.begin(); it != requirements.end(); it++) {
    s += it->getRequirement();
    if (next(it) == requirements.end()) {
      s += "]";
    }
    else {
      s += ", ";
    }
  }
  return s;
}

string OptionDict::requireOption(const string& name, const string& value, const string& comparator) {
  return requireOptions({OptionRequirement(name, value, comparator)});
}

string OptionDict::requireDdPackage(const string& ddPackageArg) {
  assert(DD_PACKAGES.contains(ddPackageArg));
  return requireOption(DD_PACKAGE_OPTION, ddPackageArg);
}

string OptionDict::helpDdPackage() {
  string s = "diagram package: ";
  for (auto it = DD_PACKAGES.begin(); it != DD_PACKAGES.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != DD_PACKAGES.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

string OptionDict::helpLogBound() {
  string s = "log10(bound) for pruning";
  s += requireOptions({
    OptionRequirement(PROJECTED_COUNTING_OPTION, "0"),
    OptionRequirement(EXIST_RANDOM_OPTION, "1"),
    OptionRequirement(LOG_COUNTING_OPTION, "1")
  });
  return s + "; float";
}

string OptionDict::helpThresholdModel() {
  string s = "threshold model for pruning";
  s += requireOptions({
    OptionRequirement(PROJECTED_COUNTING_OPTION, "0"),
    OptionRequirement(EXIST_RANDOM_OPTION, "1"),
    OptionRequirement(LOG_COUNTING_OPTION, "1"),
    OptionRequirement(LOG_BOUND_OPTION, "-inf")
  });
  return s + "; string";
}

string OptionDict::helpSatSolverPruning() {
  string s = "SAT pruning with CryptoMiniSat";
  s += requireOptions({
    OptionRequirement(PROJECTED_COUNTING_OPTION, "0"),
    OptionRequirement(EXIST_RANDOM_OPTION, "1"),
    OptionRequirement(LOG_COUNTING_OPTION, "1"),
    OptionRequirement(LOG_BOUND_OPTION, "-inf"),
    OptionRequirement(THRESHOLD_MODEL_OPTION, "\"\""),
  });
  return s + ": 0, 1; int";
}

string OptionDict::helpMaximizerFormat() {
  string s = "maximizer format";
  s += requireOptions({
    OptionRequirement(EXIST_RANDOM_OPTION, "1"),
    OptionRequirement(DD_PACKAGE_OPTION, CUDD_PACKAGE)
  });
  s += ": ";
  for (auto it = MAXIMIZER_FORMATS.begin(); it != MAXIMIZER_FORMATS.end(); it++) {
    s += to_string(it->first) + "/" + it->second;
    if (next(it) != MAXIMIZER_FORMATS.end()) {
      s += ", ";
    }
  }
  return s + "; int";
}

string OptionDict::helpSubstitutionMaximization() {
  string s = "substitution-based maximization";
  s += requireOptions({
    OptionRequirement(WEIGHTED_COUNTING_OPTION, "0"),
    OptionRequirement(MAXIMIZER_FORMAT_OPTION, to_string(NEITHER_FORMAT), ">")
  });
  return s + ": 0, 1; int";
}

string OptionDict::helpDiagramVarOrderHeuristic() {
  return "diagram var order" + util::helpVarOrderHeuristic(CNF_VAR_ORDER_HEURISTICS);
}

string OptionDict::helpSliceVarOrderHeuristic() {
  string s = "slice var order";
  s += requireOption(THREAD_SLICE_COUNT_OPTION, "1", ">");
  s += util::helpVarOrderHeuristic(util::getVarOrderHeuristics());
  return s;
}

string OptionDict::helpJoinPriority() {
  string s = "join priority: ";
  for (auto it = JOIN_PRIORITIES.begin(); it != JOIN_PRIORITIES.end(); it++) {
    s += it->first + "/" + it->second;
    if (next(it) != JOIN_PRIORITIES.end()) {
      s += ", ";
    }
  }
  return s + "; string";
}

string OptionDict::helpDynamicVarOrdering() {
  return "dynamic variable ordering. DD_PACKAGE must be CUDD. 0/1. Default 0.";
}

string OptionDict::helpSatFilter() {
  return "0 - Disable SatFilter (Only Executor) / 1 - Only SatFilter / 2 - SatFilter + Executor. Default 0.";
}

void OptionDict::runCommand() const {
  if (verboseSolving >= 1) {
    cout << "c processing command-line options...\n";
    printRow("cnfFile", cnfFilePath);
    printRow("weightedCounting", weightedCounting);
    printRow("projectedCounting", projectedCounting);
    printRow("existRandom", existRandom);
    printRow("diagramPackage", DD_PACKAGES.at(ddPackage));
    if (ddPackage == CUDD_PACKAGE) {
      printRow("logCounting", logCounting);
      printRow("dynamic var ordering", dynVarOrdering);
    }
    if (!projectedCounting && existRandom && logCounting) {
      if (logBound > -INF) {
        printRow("logBound", logBound);
      }
      else if (!thresholdModel.empty()) {
        printRow("thresholdModel", thresholdModel);
      }
      else if (satSolverPruning) {
        printRow("satSolverPruning", satSolverPruning);
      }
    }
    if (existRandom && ddPackage == CUDD_PACKAGE) {
      printRow("maximizerFormat", MAXIMIZER_FORMATS.at(maximizerFormat));
    }
    if (maximizerFormat) {
      printRow("maximizerVerification", maximizerVerification);
    }
    if (!weightedCounting && maximizerFormat) {
      printRow("substitutionMaximization", substitutionMaximization);
    }
    printRow("plannerWaitSeconds", plannerWaitDuration);
    printRow("threadCount", threadCount);
    if (ddPackage == CUDD_PACKAGE) {
      printRow("threadSliceCount", threadSliceCount);
    }
    printRow("randomSeed", randomSeed);
    printRow("diagramVarOrderHeuristic", (ddVarOrderHeuristic < 0 ? "INVERSE_" : "") + CNF_VAR_ORDER_HEURISTICS.at(abs(ddVarOrderHeuristic)));
    if (ddPackage == CUDD_PACKAGE) {
      printRow("sliceVarOrderHeuristic", (sliceVarOrderHeuristic < 0 ? "INVERSE_" : "") + util::getVarOrderHeuristics().at(abs(sliceVarOrderHeuristic)));
      printRow("memSensitivityMegabytes", memSensitivity);
    }
    printRow("maxMemMegabytes", maxMem);
    if (ddPackage == SYLVAN_PACKAGE) {
      printRow("tableRatio", tableRatio);
      printRow("initRatio", initRatio);
      printRow("multiplePrecision", multiplePrecision);
    }
    printRow("joinPriority", JOIN_PRIORITIES.at(joinPriority));
    cout << "\n";
  }

  try {
    JoinNode::cnf.readCnfFile(cnfFilePath);

    if (JoinNode::cnf.clauses.empty()) {
      cout << WARNING << "empty CNF\n";
      Executor::printAdjustedSolutionRows(logCounting ? Number() : Number("1"));
      return;
    }

    JoinTreeProcessor joinTreeProcessor(plannerWaitDuration);

    Map<Int, Number> unprunableWeights = JoinNode::cnf.getUnprunableWeights();
    if (!unprunableWeights.empty() && (logBound > -INF || !thresholdModel.empty() || satSolverPruning)) {
      JoinTreeProcessor::killPlanner();
      cout << "\n";
      cout << "c unprunable literal weights:\n";
      for (const auto& [literal, weight] : unprunableWeights) {
        JoinNode::cnf.printLiteralWeight(literal, weight);
      }
      throw MyError("must not prune if there are unprunable weights");
    }

    if (ddPackage == SYLVAN_PACKAGE) { // initializes Sylvan
      lace_init(threadCount, 0);
      lace_startup(0, NULL, NULL);
      sylvan::sylvan_set_limits(maxMem * MEGA, tableRatio, initRatio);
      sylvan::sylvan_init_package();
      sylvan::sylvan_init_mtbdd();
      if (multiplePrecision) {
        sylvan::gmp_init();
      }
    }
    if (satFilter > 0){
      SatFilter satfilter(joinTreeProcessor.getJoinTreeRoot(), ddVarOrderHeuristic);
    }
    if (satFilter != 1) {
      Executor executor(joinTreeProcessor.getJoinTreeRoot(), ddVarOrderHeuristic, sliceVarOrderHeuristic);
    }
    
    if (ddPackage == SYLVAN_PACKAGE) { // quits Sylvan
      sylvan::sylvan_quit();
      lace_exit();
    }
  }
  catch (UnsatException) {
    Executor::printAdjustedSolutionRows(logCounting ? Number(-INF) : Number(), true);
  }
}

OptionDict::OptionDict(int argc, char** argv) {
  cxxopts::Options options("dmc", "Diagram Model Counter (reads join tree from stdin)");
  options.set_width(118);

  using cxxopts::value;
  options.add_options()
    (CNF_FILE_OPTION, "CNF file path; string (required)", value<string>())
    (WEIGHTED_COUNTING_OPTION, "weighted counting: 0, 1; int", value<Int>()->default_value("1"))
    (PROJECTED_COUNTING_OPTION, "projected counting (graded join tree): 0, 1; int", value<Int>()->default_value("0"))
    (EXIST_RANDOM_OPTION, "exist-random SAT (max-sum instead of sum-max): 0, 1; int", value<Int>()->default_value("0"))
    (DD_PACKAGE_OPTION, helpDdPackage(), value<string>()->default_value(CUDD_PACKAGE))
    (LOG_COUNTING_OPTION, "logarithmic counting" + requireDdPackage(CUDD_PACKAGE) + ": 0, 1; int", value<Int>()->default_value("0"))
    (LOG_BOUND_OPTION, helpLogBound(), value<string>()->default_value(to_string(-INF))) // cxxopts fails to parse "-inf" as Float
    (THRESHOLD_MODEL_OPTION, helpThresholdModel(), value<string>()->default_value(""))
    (SAT_SOLVER_PRUNING, helpSatSolverPruning(), value<Int>()->default_value("0"))
    (MAXIMIZER_FORMAT_OPTION, helpMaximizerFormat(), value<Int>()->default_value(to_string(NEITHER_FORMAT)))
    (MAXIMIZER_VERIFICATION_OPTION, "maximizer verification" + requireOption(MAXIMIZER_FORMAT_OPTION, to_string(NEITHER_FORMAT), ">") + ": 0, 1; int", value<Int>()->default_value("0"))
    (SUBSTITUTION_MAXIMIZATION_OPTION, helpSubstitutionMaximization(), value<Int>()->default_value("0"))
    (PLANNER_WAIT_OPTION, "planner wait duration minimum (in seconds); float", value<Float>()->default_value("0.0"))
    (THREAD_COUNT_OPTION, "thread count [or 0 for hardware_concurrency value]; int", value<Int>()->default_value("1"))
    (THREAD_SLICE_COUNT_OPTION, "thread slice count" + requireDdPackage(CUDD_PACKAGE) + "; int", value<Int>()->default_value("1"))
    (RANDOM_SEED_OPTION, "random seed; int", value<Int>()->default_value("0"))
    (DYN_ORDER_OPTION, helpDynamicVarOrdering(), value<Int>()->default_value("0"))
    (SAT_FILTER_OPTION, helpSatFilter(), value<Int>()->default_value("0"))
    (DD_VAR_OPTION, helpDiagramVarOrderHeuristic(), value<Int>()->default_value(to_string(MCS_HEURISTIC)))
    (SLICE_VAR_OPTION, helpSliceVarOrderHeuristic(), value<Int>()->default_value(to_string(BIGGEST_NODE_HEURISTIC)))
    (MEM_SENSITIVITY_OPTION, "memory sensitivity (in MB) for reporting usage" + requireDdPackage(CUDD_PACKAGE) + "; float", value<Float>()->default_value("1e3"))
    (MAX_MEM_OPTION, "maximum memory (in MB) for unique table and cache table combined [or 0 for unlimited memory with CUDD]; float", value<Float>()->default_value("4e3"))
    (TABLE_RATIO_OPTION, "table ratio" + requireDdPackage(SYLVAN_PACKAGE) + ": log2(unique_size/cache_size); int", value<Int>()->default_value("1"))
    (INIT_RATIO_OPTION, "init ratio for tables" + requireDdPackage(SYLVAN_PACKAGE) + ": log2(max_size/init_size); int", value<Int>()->default_value("10"))
    (MULTIPLE_PRECISION_OPTION, "multiple precision" + requireDdPackage(SYLVAN_PACKAGE) + ": 0, 1; int", value<Int>()->default_value("0"))
    (JOIN_PRIORITY_OPTION, helpJoinPriority(), value<string>()->default_value(SMALLEST_PAIR))
    (VERBOSE_CNF_OPTION, util::helpVerboseCnfProcessing(), value<Int>()->default_value("0"))
    (VERBOSE_JOIN_TREE_OPTION, "verbose join-tree processing: 0, 1, 2", value<Int>()->default_value("0"))
    (VERBOSE_PROFILING_OPTION, "verbose profiling: 0, 1, 2; int", value<Int>()->default_value("0"))
    (VERBOSE_SOLVING_OPTION, util::helpVerboseSolving(), value<Int>()->default_value("0"))
    (HELP_OPTION, "help")
  ;

  cxxopts::ParseResult result = options.parse(argc, argv);
  if (result.count(HELP_OPTION) || !result.count(CNF_FILE_OPTION)) {
    cout << options.help();
  }
  else {
    cnfFilePath = result[CNF_FILE_OPTION].as<string>();

    weightedCounting = result[WEIGHTED_COUNTING_OPTION].as<Int>(); // global var

    projectedCounting = result[PROJECTED_COUNTING_OPTION].as<Int>(); // global var

    existRandom = result[EXIST_RANDOM_OPTION].as<Int>(); // global var

    ddPackage = result[DD_PACKAGE_OPTION].as<string>(); // global var
    assert(DD_PACKAGES.contains(ddPackage));

    logCounting = result[LOG_COUNTING_OPTION].as<Int>(); // global var
    assert(!logCounting || ddPackage == CUDD_PACKAGE);

    logBound = stold(result[LOG_BOUND_OPTION].as<string>()); // global var
    assert(logBound == -INF || !projectedCounting);
    assert(logBound == -INF || existRandom);
    assert(logBound == -INF || logCounting);

    thresholdModel = result[THRESHOLD_MODEL_OPTION].as<string>(); // global var
    assert(thresholdModel.empty() || !projectedCounting);
    assert(thresholdModel.empty() || existRandom);
    assert(thresholdModel.empty() || logCounting);
    assert(thresholdModel.empty() || logBound == -INF);

    satSolverPruning = result[SAT_SOLVER_PRUNING].as<Int>(); // global var
    assert(!satSolverPruning || !projectedCounting);
    assert(!satSolverPruning || existRandom);
    assert(!satSolverPruning || logCounting);
    assert(!satSolverPruning || logBound == -INF);
    assert(!satSolverPruning || thresholdModel.empty());

    maximizerFormat = result[MAXIMIZER_FORMAT_OPTION].as<Int>(); // global var
    assert(MAXIMIZER_FORMATS.contains(maximizerFormat));
    assert(!maximizerFormat || existRandom);
    assert(!maximizerFormat || ddPackage == CUDD_PACKAGE);

    maximizerVerification = result[MAXIMIZER_VERIFICATION_OPTION].as<Int>(); // global var
    assert(!maximizerVerification || maximizerFormat);

    substitutionMaximization = result[SUBSTITUTION_MAXIMIZATION_OPTION].as<Int>(); // global var
    assert(!substitutionMaximization || !weightedCounting);
    assert(!substitutionMaximization || maximizerFormat);

    plannerWaitDuration = result[PLANNER_WAIT_OPTION].as<Float>();
    plannerWaitDuration = max(plannerWaitDuration, 0.0l);

    threadCount = result[THREAD_COUNT_OPTION].as<Int>(); // global var
    if (threadCount <= 0) {
      threadCount = thread::hardware_concurrency();
    }
    assert(threadCount > 0);

    threadSliceCount = result[THREAD_SLICE_COUNT_OPTION].as<Int>(); // global var
    threadSliceCount = max(threadSliceCount, 1ll);
    assert(threadSliceCount == 1 || ddPackage == CUDD_PACKAGE);

    randomSeed = result[RANDOM_SEED_OPTION].as<Int>(); // global var
    
    dynVarOrdering = result[DYN_ORDER_OPTION].as<Int>();
    assert(dynVarOrdering == 0 || ddPackage == CUDD_PACKAGE);

    satFilter = result[SAT_FILTER_OPTION].as<Int>();
    assert(satFilter >= 0 && satFilter <=2);

    ddVarOrderHeuristic = result[DD_VAR_OPTION].as<Int>();
    assert(CNF_VAR_ORDER_HEURISTICS.contains(abs(ddVarOrderHeuristic)));

    assert(!result.count(SLICE_VAR_OPTION) || threadSliceCount > 1);
    sliceVarOrderHeuristic = result[SLICE_VAR_OPTION].as<Int>();
    assert(util::getVarOrderHeuristics().contains(abs(sliceVarOrderHeuristic)));

    assert(!result.count(MEM_SENSITIVITY_OPTION) || ddPackage == CUDD_PACKAGE);
    memSensitivity = result[MEM_SENSITIVITY_OPTION].as<Float>(); // global var

    maxMem = result[MAX_MEM_OPTION].as<Float>(); // global var
    maxMem = max(maxMem, 0.0l);

    assert(!result.count(TABLE_RATIO_OPTION) || ddPackage == SYLVAN_PACKAGE);
    tableRatio = result[TABLE_RATIO_OPTION].as<Int>();

    assert(!result.count(INIT_RATIO_OPTION) || ddPackage == SYLVAN_PACKAGE);
    initRatio = result[INIT_RATIO_OPTION].as<Int>();

    multiplePrecision = result[MULTIPLE_PRECISION_OPTION].as<Int>(); // global var
    assert(!multiplePrecision || ddPackage == SYLVAN_PACKAGE);

    joinPriority = result[JOIN_PRIORITY_OPTION].as<string>(); //global var
    assert(JOIN_PRIORITIES.contains(joinPriority));

    verboseCnf = result[VERBOSE_CNF_OPTION].as<Int>(); // global var

    verboseJoinTree = result[VERBOSE_JOIN_TREE_OPTION].as<Int>(); // global var

    verboseProfiling = result[VERBOSE_PROFILING_OPTION].as<Int>(); // global var
    assert(verboseProfiling <= 0 || threadCount == 1);

    verboseSolving = result[VERBOSE_SOLVING_OPTION].as<Int>(); // global var

    toolStartPoint = util::getTimePoint(); // global var
    runCommand();
    printRow("seconds", util::getDuration(toolStartPoint));
  }
}

/* global functions ========================================================= */

int main(int argc, char** argv) {
  cout << std::unitbuf; // enables automatic flushing
  OptionDict(argc, argv);
}
