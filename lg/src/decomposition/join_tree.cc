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
int create_grade(
  const util::GradedClauses &graded_clauses,
  const util::Formula &formula,
  const TreeDecomposition &tree_decomposition,
  JoinTree *result,
  const int starting_node
) {
  const std::vector<util::GradedClauses> &clauses = graded_clauses.components();

  // Record the clauses that each variable appears in
  std::vector<std::vector<size_t>> appearances(formula.num_variables()+1);
  for (size_t i = 0; i < clauses.size(); i++) {
    for (size_t var : clauses[i].variables()) {
      appearances[var].push_back(i);
    }
  }
  std::vector<bool> found(clauses.size(), false);

  // Build the structure of the join tree from the tree decomposition.
  int root = tree_decomposition.visit<int>(starting_node, [&] (
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
                          clauses[clause_id].variables().begin(),
                          clauses[clause_id].variables().end())) {
          int below;
          if (clauses[clause_id].clause_id() == SIZE_MAX) {
            below = create_grade(clauses[clause_id],
                                 formula,
                                 tree_decomposition,
                                 result,
                                 node.id);
          } else {
            below = result->add_leaf(clauses[clause_id].clause_id());
          }
          children.push_back(below);
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
      return static_cast<int>(result->add_internal(children));
    }
  });

  // Handle clauses with no variables
  std::vector<int> disjoint_components(0);
  for (size_t i = 0; i < clauses.size(); i++) {
    if (clauses[i].variables().size() == 0) {
      if (clauses[i].clause_id() == SIZE_MAX) {
        disjoint_components.push_back(
          create_grade(clauses[i],
                      formula,
                      tree_decomposition,
                      result,
                      starting_node));
      } else {
        // Assert: Empty clauses should be virtual clauses incurred by grading
        std::cerr << "Reduction error: Empty clause" << std::endl;
        return -1;
      }
    }
  }
  if (disjoint_components.size() > 0) {
    // Connect all disjoint components at the root, if they exist
    disjoint_components.push_back(root);
    root = result->add_internal(disjoint_components);
  }

  // Ensure that free variables are kept until at least this point
  if (graded_clauses.variables().size() == 0) {
    return root;
  } else {
    return result->add_downgrade(root, graded_clauses.variables());
  }
}


std::optional<JoinTree> JoinTree::graded_from_tree_decomposition(
  const util::GradedClauses &graded_clauses,
  const util::Formula &formula,
  const TreeDecomposition &tree_decomposition) {
  JoinTree result;
  int root = create_grade(graded_clauses,
                          formula,
                          tree_decomposition,
                          &result,
                          1);
  if (!result.set_root(static_cast<size_t>(root))) {
    return std::nullopt;  // Unable to set root.
  }
  result.compute_projected_variables(formula);
  result.compute_width(formula);
  return result;
}

void JoinTree::compute_projected_variables(const util::Formula &formula) {
  highest_projected_var_ = formula.num_variables();
  auto visitor = [&] (const JoinTreeNode &node,
                      std::vector<std::vector<int>> children) {
    if (children.size() == 1) {
      // Ensure that forced variables are kept until at least this point
      for (size_t var : node.forced_variables) {
        children[0][var] = node.node_id;
      }
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
        return static_cast<size_t>(1);
      } else {
        // A dummy node will be added later
        return static_cast<size_t>(2);
      }
    }

    size_t result = std::accumulate(children.begin(), children.end(), 0);
    // Nodes with 1 child and no projections will be skipped
    if (children.size() > 1 || node.projected_variables.size() > 0) {
      result += 1;
    }
    return result;
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
    if (children.size() == 1 && node.projected_variables.size() == 0) {
      return children[0];  // Skip nodes with 1 child and 0 projections
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

  // Print out the join tree width
  *output << "c joinTreeWidth " << width_ << "\n";
}

void JoinTree::compute_width(const util::Formula &formula) {
  width_ = 0;
  visit<std::vector<size_t>>([&] (const JoinTreeNode &node,
                                  std::vector<std::vector<size_t>> children) {
    if (children.size() == 0 && node.projected_variables.size() == 0) {
      // Short-circuit for simple leaf nodes
      width_ = std::max(width_,
                        formula.clause_variables()[node.clause_id].size());
      return formula.clause_variables()[node.clause_id];
    }

    std::vector<size_t> vars;
    if (children.size() == 0) {
      vars.insert(vars.end(),
                  formula.clause_variables()[node.clause_id].begin(),
                  formula.clause_variables()[node.clause_id].end());
    } else {
      // Combine the variables from all children
      for (std::vector<size_t> &child : children) {
        vars.insert(vars.end(), child.begin(), child.end());
      }
      // (Remove duplicates in list of variables)
      std::sort(vars.begin(), vars.end());
      vars.erase(std::unique(vars.begin(), vars.end()),
                 vars.end());
    }

    width_ = std::max(width_, vars.size());
    // Remove all projected variables
    vars.erase(set_difference(vars.begin(), vars.end(),
                              node.projected_variables.begin(),
                              node.projected_variables.end(),
                              vars.begin()),
                vars.end());
    return vars;
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

size_t JoinTree::add_downgrade(int child, const std::vector<size_t> &forced) {
  JoinTreeNode &info = tree_.add_vertex(num_nodes_);
  info.node_id = num_nodes_;
  info.forced_variables = forced;
  tree_.add_edge(child, num_nodes_);
  num_nodes_++;
  return num_nodes_-1;
}
}  // namespace decomposition
