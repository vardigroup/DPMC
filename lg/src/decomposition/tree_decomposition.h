/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <vector>

#include "util/formula.h"
#include "decomposition/tree.h"

/**
 * This file contains various classes required to implement a tree
 * decomposition.
 */

namespace decomposition {
/**
 * A simple class which holds the (sorted) bag at each node of the tree
 * decomposition.
 */
struct TreeDecompositionNode {
 public:
  std::vector<size_t> bag;
  int id;
};

/**
 * A class represeting a tree decomposition.
 */
class TreeDecomposition : public Tree<TreeDecompositionNode> {
 public:
  /**
   * Compute the treewidth of this tree decomposition.
   */
  int compute_treewidth() const;

  /**
   * Parse a single tree decomposition from the provided input stream.
   * (Until an '=' line is reached).
   */
  static std::optional<TreeDecomposition> parse_one(std::istream *stream);
};

}  // namespace decomposition
