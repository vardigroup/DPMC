/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <string>
#include <vector>

namespace util {
class GradedClauses {
 public:
  GradedClauses() {}
  GradedClauses(std::vector<size_t> variables, size_t clause_id)
  : variables_(variables), clause_id_(clause_id) {}
  explicit GradedClauses(const std::vector<std::vector<size_t>> &clauses);

  /**
   * Group clauses according to the provided variables,
   * as if the provided variables must be projected out second
   */
  void group_by(const std::vector<size_t> &kept_variables, size_t max_var_id);

  /**
   * Output the line graph of this clause set.
   */
  void write_line_graph(std::ostream *output, size_t num_variables) const;

  size_t clause_id() const {
    return clause_id_;
  }

  const std::vector<GradedClauses> &components() const {
    return components_;
  }

  const std::vector<size_t> &variables() const {
    return variables_;
  }

 private:
  size_t count_line_graph_edges() const;
  void write_line_graph_edges(std::ostream *output) const;

  std::vector<GradedClauses> components_ = {};
  std::vector<size_t> variables_ = {};
  size_t clause_id_ = SIZE_MAX;
};
}  // namespace util
