/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include "decomposition/join_tree.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <numeric>
#include <unordered_set>
#include <set>
#include <vector>

namespace decomposition {

std::optional<JoinTree> JoinTree::from_tree_decomposition(
  const util::Formula &formula,
  const TreeDecomposition &tree_decomposition) {
  JoinTree result;

  // Record the clauses that each variable appears in
  std::vector<std::vector<size_t>> appearances(formula.num_variables()+1);
  for (size_t i = 0; i < formula.clause_variables().size(); i++) {
    for (size_t var : formula.clause_variables()[i]) {
      appearances[var].push_back(i);
    }
  }
  std::vector<bool> found(formula.clause_variables().size(), false);

  // Build the structure of the join tree from the tree decomposition.
  int root = tree_decomposition.visit<int>(1, [&] (
    const TreeDecompositionNode &node,
    std::vector<int> children) {

    // Remove all children with no matching clauses.
    children.erase(std::remove(children.begin(), children.end(), -1),
                   children.end());

    // Determine all clauses whose variables are entirely contained in this bag.
    // Such clauses are added as a leaf here in the join tree.
    for (size_t variable : node.bag) {
      for (size_t clause_id : appearances[variable]) {
        if (found[clause_id]) {
          continue;
        }

        if (std::includes(node.bag.begin(),
                          node.bag.end(),
                          formula.clause_variables()[clause_id].begin(),
                          formula.clause_variables()[clause_id].end())) {
          children.push_back(result.add_leaf(clause_id));
          found[clause_id] = true;
        }
      }
    }

    // Build the corresponding internal join node
    if (children.size() == 0) {
      return -1;  // No node needed; no children match to clauses.
    } else if (children.size() == 1) {
      return children[0];  // No join needed.
    } else {
      return static_cast<int>(result.add_internal(children));
    }
  });
  if (!result.set_root(static_cast<size_t>(root))) {
    return std::nullopt;  // Unable to set root.
  }
  result.compute_projected_variables(formula);
  return result;
}

void JoinTree::compute_projected_variables(const util::Formula &formula) {
  highest_projected_var_ = formula.num_variables();
  auto visitor = [&] (const JoinTreeNode &node,
                      std::vector<std::vector<int>> children) {
    if (children.size() == 1) {
      return children[0];
    }

    std::vector<int> result(highest_projected_var_+1, -1);
    if (children.size() == 0) {
      for (size_t var : formula.clause_variables()[node.clause_id]) {
        result[var] = node.node_id;
      }
      return result;
    } else {
      for (std::vector<int> &child : children) {
        for (size_t i = 1; i <= highest_projected_var_; i++) {
          if (child[i] != -1) {
            if (result[i] == -1) {
              result[i] = child[i];
            } else {
              result[i] = node.node_id;
            }
          }
        }
      }
      return result;
    }
  };

  // Clear any old projected variables.
  for (size_t i = 0; i < num_nodes_; i++) {
    JoinTreeNode *info = tree_[i];
    if (info != nullptr) {
      tree_[i]->projected_variables.clear();
    }
  }

  std::vector<int> result = visit<std::vector<int>>(visitor);
  for (size_t i = 1; i <= highest_projected_var_; i++) {
    if (result[i] != -1) {
      JoinTreeNode *info = tree_[result[i]];
      if (info != nullptr) {
        info->projected_variables.push_back(i);
      }
    }
  }
}

void JoinTree::write(std::ostream *output) const {
  // The .jt format uses dummy nodes if clauses have projected variables.
  // Compute the total number of nodes in the tree including these dummy nodes.
  size_t num_nodes = visit<size_t>([&](const JoinTreeNode &node,
                                       std::vector<size_t> children) {
    if (children.size() == 0) {
      if (node.projected_variables.size() == 0) {
        return 1;
      } else {
        return 2;
      }
    }

    return 1 + std::accumulate(children.begin(), children.end(), 0);
  });

  // Write the join tree header
  *output << "p jt";
  *output << " " << highest_projected_var_;
  *output << " " << highest_leaf_id_+1;
  *output << " " << num_nodes;
  *output << "\n";

  // Print out all internal nodes
  size_t next_id = highest_leaf_id_+2;
  visit<size_t>([&](const JoinTreeNode &node,
                 std::vector<size_t> children) {
    if (children.size() == 0 && node.projected_variables.size() == 0) {
      return node.clause_id+1;  // CNF clauses are numbered 1 to M
    }

    if (children.size() == 0) {
      // There are variables to project at this leaf node.
      // Add a dummy internal node for the projection.
      children.push_back(node.clause_id+1);
    }  // Fall-through

    *output << next_id;
    for (size_t child : children) {
      *output << " " << child;
    }
    *output << " e";
    for (size_t projected : node.projected_variables) {
      *output << " " << projected;
    }
    *output << "\n";
    next_id++;
    return next_id-1;
  });
}

size_t JoinTree::add_leaf(size_t clause_id) {
  JoinTreeNode &info = tree_.add_vertex(num_nodes_);
  info.clause_id = clause_id;
  info.node_id = num_nodes_;
  if (highest_leaf_id_ < clause_id) {
    highest_leaf_id_ = clause_id;
  }
  num_nodes_++;
  return num_nodes_-1;
}

size_t JoinTree::add_internal(const std::vector<int> &children) {
  JoinTreeNode &info = tree_.add_vertex(num_nodes_);
  info.node_id = num_nodes_;
  for (int child : children) {
    tree_.add_edge(child, num_nodes_);
  }
  num_nodes_++;
  return num_nodes_-1;
}
}  // namespace decomposition
