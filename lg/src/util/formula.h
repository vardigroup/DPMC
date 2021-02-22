/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "util/graded_clauses.h"

namespace util {
/**
 * Represents a boolean formula in CNF with literal weights.
 */
class Formula {
 public:
  explicit Formula(int num_variables) : num_variables_(num_variables) {}

  /**
   * Adds a CNF clause to the formula containing the provided literals.
   *
   * Returns true if all literals are valid (either an identifier returned 
   * by add_variable or the negation of an identifier), and false otherwise.
   */
  bool add_clause(std::vector<int> literals);

  /**
   * Get the set of clauses, graded according to the relevant variables.
   */
  util::GradedClauses graded_clauses();

  int num_variables() const { return num_variables_; }

  /**
   * Get the (sorted) sets of variables in each clause.
   */
  const std::vector<std::vector<size_t>> &clause_variables() const {
    return clause_variables_;
  }

  /*
  * Parses a file in DIMACS format into a boolean formula.
  *
  * Returns the parsed formula if the DIMACS file is in a valid format.
  */
  static std::optional<Formula> parse_DIMACS(std::istream *stream);

 private:
  // Number of variables in the formula
  size_t num_variables_ = 0;
  // Set of clauses
  std::vector<std::vector<int>> clauses_ = {};
  // Set of variables in each clause (sorted)
  std::vector<std::vector<size_t>> clause_variables_ = {};


  // Set of relevant variables
  std::vector<size_t> relevant_vars_;
};
}  // namespace util
