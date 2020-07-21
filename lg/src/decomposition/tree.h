/******************************************
Copyright (c) 2020, Jeffrey Dudek
******************************************/

#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/graph_traits.hpp>

/**
 * This file contains various classes required to implement a tree using boost,
 * including a visitor on trees.
 */

namespace decomposition {
template<typename NodeBase, typename Result>
using Visitor = std::function<Result(const NodeBase &, std::vector<Result>)>;

template<typename NodeBase>
using Graph = typename boost::adjacency_list<boost::vecS, boost::vecS,
                                                 boost::undirectedS, NodeBase>;

template<typename NodeBase>
using arc_t = typename boost::graph_traits<Graph<NodeBase>>::edge_descriptor;

template<typename NodeBase>
using node_t = typename boost::graph_traits<Graph<NodeBase>>::vertex_descriptor;

/**
 * A class represeting a tree.
 */
template<typename NodeBase>
class Tree {
 public:
  /**
   * Adds "key" as a node in the tree (if it is not already). 
   * 
   * Returns a reference to the bag stored at the node "key".
   */
  NodeBase &add_vertex(int key) {
    node_t<NodeBase> vertex;

    auto previous_vertex = vertices_by_id_.find(key);
    if (previous_vertex != vertices_by_id_.end()) {
      vertex = previous_vertex->second;
    } else {
      vertex = boost::add_vertex(base_);
      vertices_by_id_.emplace(key, vertex);
    }

    return base_[vertex];
  }

  /**
   * Adds an edge between the two provided nodes and returns true if both nodes
   * exist in the tree. Otherwise, returns false.
   * 
   * This method does NOT force the graph to remain a tree.
   */
  bool add_edge(int key1, int key2) {
    try {
      boost::add_edge(vertices_by_id_.at(key1),
                      vertices_by_id_.at(key2),
                      base_);
      return true;
    } catch (std::out_of_range) {
      return false;  // invalid TensorIndex argument
    }
  }

  /**
   * Returns true if this is actually a tree, and false otherwise.
   * 
   * (In particular, ensures that the tree is a connected acyclic graph)
   */
  bool validate() const {
    size_t num_vertices = vertices_by_id_.size();
    std::vector<bool> seen(num_vertices, false);
    size_t seen_count = 0;

    std::function<bool(node_t<NodeBase>, node_t<NodeBase>)> has_cycle =
    [&] (node_t<NodeBase> parent, node_t<NodeBase> current) {
      if (seen[current]) {
        return true;
      }
      seen[current] = true;
      ++seen_count;
      auto vs = boost::adjacent_vertices(current, base_);
      for (auto neighbor = vs.first; neighbor != vs.second; ++neighbor) {
        if (*neighbor == parent)
          continue;
        if (has_cycle(current, *neighbor))
          return true;
      }
      return false;
    };

    return !has_cycle(0, 0) && (seen_count == num_vertices);
  }

  /**
   * Runs "visitor" on every node in the tree in a postorder traversal from "root",
   * ultimately returning the value of "visitor" on root.
   * 
   * Each visitor call is provided the information at the current node, 
   * and the results of the visitor call on its children.
   */
  template<typename Result>
  Result visit(int root, const Visitor<NodeBase, Result> &visitor) const {
    size_t root_id = vertices_by_id_.at(root);
    return visit_helper(root_id, root_id, visitor);
  }


  /**
   * Access the data stored at the indicated vertex.
   */
  NodeBase * operator[](int key) {
    auto vertex = vertices_by_id_.find(key);
    if (vertex != vertices_by_id_.end()) {
      return &base_[vertex->second];
    } else {
      return nullptr;
    }
  }

 protected:
  // The underlying graph.
  Graph<NodeBase> base_;
  // A map from node keys to node ids in the underlying graph.
  std::unordered_map<int, node_t<NodeBase>> vertices_by_id_ = {};

 private:
  /**
   * A helper function for ::visit with a more convenient signature.
   */
  template<typename Result>
  Result visit_helper(node_t<NodeBase> parent, node_t<NodeBase> current,
                      const Visitor<NodeBase, Result> &visitor) const {
    std::vector<Result> child_results;
    auto vs = boost::adjacent_vertices(current, base_);
    for (auto neighbor = vs.first; neighbor != vs.second; ++neighbor) {
      if (*neighbor == parent)
        continue;
      child_results.push_back(visit_helper(current, *neighbor, visitor));
    }

    return visitor(base_[current], std::move(child_results));
  }
};
}  // namespace decomposition
