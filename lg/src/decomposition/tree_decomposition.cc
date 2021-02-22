/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#include "decomposition/tree_decomposition.h"

#include <algorithm>
#include <string>

#include "util/dimacs_parser.h"

namespace decomposition {
int TreeDecomposition::compute_treewidth() const {
  size_t max_size = 1;
  auto vs = boost::vertices(base_);
  for (auto v = vs.first; v != vs.second; ++v) {
    if (base_[*v].bag.size() > max_size) {
      max_size = base_[*v].bag.size();
    }
  }
  return static_cast<int>(max_size) - 1;
}

std::optional<TreeDecomposition> TreeDecomposition::parse_one(
  std::istream *stream) {
  util::DimacsParser parser(stream, &std::cout);

  // Parse the header
  std::vector<double> entries;
  if (!parser.parseExpectedLine("s td", &entries) || entries.size() != 3) {
    return std::nullopt;
  }

  TreeDecomposition result;
  int num_bags = entries[0];
  int max_bag_size = entries[1];
  int max_bag_entry = entries[2];

  // Parse the remaining clauses
  while (!parser.finished()) {
    entries.clear();
    std::string prefix = parser.parseLine(&entries);
    if (prefix == "b") {
      if (entries.size() == 0) {
        return std::nullopt;  // Bag must have a node identifier.
      }
      if (entries.size() > max_bag_size + 1) {
        return std::nullopt;  // Bag is too large.
      }
      if (entries[0] <= 0 || entries[0] > num_bags) {
        return std::nullopt;  // Node id is not in range.
      }
      TreeDecompositionNode &node = result.add_vertex(entries[0]);
      node.id = static_cast<int>(entries[0]);
      for (size_t i = 1; i < entries.size(); i++) {
        if (entries[i] <= 0 || entries[i] > max_bag_entry) {
          return std::nullopt;  // Bad bag entry.
        }
        node.bag.push_back(entries[i]);
      }
      std::sort(node.bag.begin(), node.bag.end());
    } else if (prefix == "=") {
      return result;
    } else {
      if (entries.size() != 2) {
        return std::nullopt;  // Each edge must span two nodes.
      }
      if (entries[0] <= 0 || entries[0] > num_bags
          || entries[1] <= 0 || entries[1] > num_bags) {
        return std::nullopt;  // Node id is not in range.
      }
      result.add_edge(entries[0], entries[1]);
    }
  }

  return result;
}
}  // namespace decomposition
