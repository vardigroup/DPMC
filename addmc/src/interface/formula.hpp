#pragma once

/* inclusions *****************************************************************/

#include "graph.hpp"

/* constants ******************************************************************/

extern const string &CNF_WORD;
extern const string &WCNF_WORD;
extern const string &WPCNF_WORD;
extern const string &WEIGHTS_WORD; // MINIC2D weights line
extern const string &WEIGHT_WORD; // CACHET/WCNF/WPCNF weight lines
extern const string &LINE_END_WORD; // required for clause lines and optional for weight lines

/* classes ********************************************************************/

class Label : public vector<Int> { // for lexicographic search
public:
  void addNumber(Int i); // retains descending order
};

class Cnf {
protected:
  vector<vector<Int>> clauses;
  vector<Int> apparentVars; // vars appearing in clauses, ordered by first appearance
  Set<Int> additiveVars;

  Int declaredVarCount = DUMMY_MIN_INT;

  Map<Int, Float> literalWeights; // for additive and disjunctive vars
  WeightFormat weightFormat;

  static string getWeightFormatWord(WeightFormat weightFormat);
  void updateApparentVars(Int literal); // adds var to apparentVars
  void addClause(const vector<Int> &clause); // writes: clauses, apparentVars
  void checkLiteralWeights() const; // for non-positive numbers
  Graph getGaifmanGraph() const;
  vector<Int> getAppearanceVarOrdering() const;
  vector<Int> getDeclarationVarOrdering() const;
  vector<Int> getRandomVarOrdering() const;
  vector<Int> getMcsVarOrdering() const;
  vector<Int> getLexpVarOrdering() const;
  vector<Int> getLexmVarOrdering() const;
  vector<Int> getMinfillVarOrdering() const;

public:
  vector<Int> getVarOrdering(VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering) const;
  vector<Int> getRestrictedVarOrdering(VarOrderingHeuristic varOrderingHeuristic, bool inverseVarOrdering, const Set<Int> &restrictedVars) const;
  Int getDeclaredVarCount() const;
  Map<Int, Float> getLiteralWeights() const;
  Int getEmptyClauseIndex() const; // first (non-negative) index if found else DUMMY_MIN_INT
  const vector<vector<Int>> &getClauses() const;
  const vector<Int> &getApparentVars() const;
  const Set<Int> &getAdditiveVars() const;
  Set<Int> getDisjunctiveVars() const;
  void printAdditiveVars() const;
  void printLiteralWeights() const;
  void printClauses() const;
  Cnf(const vector<vector<Int>> &clauses = vector<vector<Int>>());
  Cnf(const string &filePath, WeightFormat weightFormat);
};
