/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <vector>

#include "decomposition/tree.h"
#include "decomposition/tree_decomposition.h"
#include "util/formula.h"
#include "util/graded_clauses.h"


/**
 * This file contains various classes required to implement a join tree.
 */

namespace decomposition {
/**
 * A simple class which holds the projected variables at each node.
 */
struct JoinTreeNode {
 public:
  size_t clause_id;
  size_t node_id;
  std::vector<size_t> projected_variables;
  std::vector<size_t> forced_variables;
};

/**
 * A class represeting a join tree.
 */
class JoinTree {
 public:
  JoinTree()
  : num_nodes_(0), highest_leaf_id_(0), root_(0)
  { }

  /**
   * Runs "visitor" on every node in the tree in a postorder traversal from
   * the root, ultimately returning the value of "visitor" on root.
   * 
   * Each visitor call is provided the information at the current node,
   * and the results of the visitor call on its children.
   */
  template<typename Result>
  Result visit(const Visitor<JoinTreeNode, Result> &visitor) const {
    return tree_.visit(root_, visitor);
  }

  /**
   * Output the join tree.
   */
  void write(std::ostream *output) const;

  /**
   * Add a leaf to the join tree.
   */
  size_t add_leaf(size_t clause_id);

  /**
   * Add an interal node to the join tree.
   */
  size_t add_internal(const std::vector<int> &children);

  /**
   * Add an downgrade node to the join tree.
   */
  size_t add_downgrade(int child, const std::vector<size_t> &forced_variables);

  /**
   * Store the projected variables implicit in joining with the provided formula.
   */
  void compute_projected_variables(const util::Formula &formula);

  /**
   * Store the width of this project-join tree.
   */
  void compute_width(const util::Formula &formula);

  /**
   * Set the root of the join tree.
   */
  bool set_root(size_t root) {
    if (root < 0 || root >= num_nodes_) {
      return false;   // Root out of range.
    }
    root_ = root;
    return true;
  }

  /**
   * Construct the join tree represented by the provided tree decomposition.
   */
  static std::optional<JoinTree> graded_from_tree_decomposition(
    const util::GradedClauses &graded_clauses,
    const util::Formula &formula,
    const decomposition::TreeDecomposition &tree_decomposition);

 private:
  size_t root_;
  size_t highest_leaf_id_;
  size_t highest_projected_var_;
  size_t num_nodes_;
  size_t width_;
  Tree<JoinTreeNode> tree_;
};
}  // namespace decomposition
