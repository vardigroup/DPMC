/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include "util/graded_clauses.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/pending/disjoint_sets.hpp>

namespace util {
GradedClauses::GradedClauses(
  const std::vector<std::vector<size_t>> &clauses
) {
  for (size_t i = 0; i < clauses.size(); i++) {
    components_.push_back(GradedClauses(clauses[i], i));
  }
}

void GradedClauses::write_line_graph(
  std::ostream *output,
  size_t num_variables
) const {
  size_t num_edges = count_line_graph_edges();
  *output << "p tw " << num_variables << " " << num_edges << "\n";
  write_line_graph_edges(output);
}

size_t GradedClauses::count_line_graph_edges() const {
  size_t total = (variables_.size() * (variables_.size()-1)) / 2;
  for (const GradedClauses &clause : components_) {
    total += clause.count_line_graph_edges();
  }
  return total;
}

void GradedClauses::write_line_graph_edges(std::ostream *output) const {
  for (size_t i = 0; i < variables_.size(); i++) {
    for (size_t j = i+1; j < variables_.size(); j++) {
      *output << variables_[i] << " ";
      *output << variables_[j] << "\n";
    }
  }
  for (const GradedClauses &clause : components_) {
    clause.write_line_graph_edges(output);
  }
}

void GradedClauses::group_by(
  const std::vector<size_t> &kept_variables,
  size_t max_var_id
) {
  /*
  Store variables to project in easily accessible way
  */
  std::vector<bool> is_projected_var(max_var_id+1, true);
  for (size_t var : kept_variables) {
    is_projected_var[var] = false;
  }

  /*
  Prepare a disjoint set data structure, whose entries are projected variables
  */
  std::vector<int> rank(is_projected_var.size());
  std::vector<int> parent(is_projected_var.size());
  boost::disjoint_sets<int *, int*> classes(&rank[0], &parent[0]);
  for (size_t var_id = 0; var_id < is_projected_var.size(); var_id++) {
    if (is_projected_var[var_id]) {
      classes.make_set(var_id);
    }
  }

  /*
  Build equivalence classes of variables for the (transitive closure of):
    x ~ y if x=y or (x and y are projected and some clause has both x and y)
  */
  for (const GradedClauses &clause : components_) {
    size_t last = SIZE_MAX;
    for (size_t var_id : clause.variables_) {
      if (is_projected_var[var_id]) {
        if (last != SIZE_MAX) {
          // var_id and last appear together in a clause, so equate them
          classes.union_set(var_id, last);
        }
        last = var_id;
      }
    }
  }

  /*
  Group clauses together if they have equivalent, projected variables
  */
  std::vector<GradedClauses> result;
  std::unordered_map<size_t, size_t> grouped_components;
  for (int i = 0; i < components_.size(); i++) {
    // Choose a GradedClauses for component i
    size_t which_group = SIZE_MAX;
    for (size_t var_id : components_[i].variables_) {
      // Clauses with a projected variable joins a group cooresponding to
      // their equivalence class
      if (is_projected_var[var_id]) {
        size_t rep = classes.find_set(var_id);
        auto group = grouped_components.find(rep);
        if (group != grouped_components.end()) {
          which_group = group->second;
        } else {
          which_group = result.size();
          result.push_back(GradedClauses());
          grouped_components.insert({rep, which_group});
        }
        break;
      }
    }

    if (which_group == SIZE_MAX) {
      // Clauses with no projected variables are placed alone
      result.push_back(std::move(components_[i]));
    } else {
      result[which_group].components_.push_back(std::move(components_[i]));
    }
  }

  /*
  Compute the sets of free variables for the new groups
  */
  for (GradedClauses &new_group : result) {
    // Clauses with no projected variables already have free variables set
    if (new_group.components_.size() == 0) {
      continue;
    }

    for (const GradedClauses &clause : new_group.components_) {
      for (size_t var_id : clause.variables_) {
        if (is_projected_var[var_id]) {
          continue;
        } else {
          new_group.variables_.push_back(var_id);
        }
      }
    }

    std::sort(new_group.variables_.begin(), new_group.variables_.end());
    new_group.variables_.erase(unique(new_group.variables_.begin(),
                                      new_group.variables_.end()),
                                      new_group.variables_.end());
  }

  components_ = std::move(result);
}
}  // namespace util
