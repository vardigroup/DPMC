#pragma once

/* inclusions *****************************************************************/

#include "util.hpp"

/* uses ***********************************************************************/

using util::printComment;
using util::printThickLine;
using util::printThinLine;
using util::showError;
using util::showWarning;

/* classes ********************************************************************/

class Graph { // undirected
protected:
  Set<Int> vertices;
  Map<Int, Set<Int>> adjacencyMap;

  bool isNeighbor(Int v1, Int v2) const;
  bool hasPath(Int v1, Int v2, Set<Int> &visitedVertices) const; // path length >= 0

public:
  bool hasPath(Int from, Int to) const;
  void printVertices() const;
  void printAdjacencyMap() const;
  Graph(const Set<Int> &vs);
  void addEdge(Int v1, Int v2);
  Set<Int>::const_iterator beginVertices() const;
  Set<Int>::const_iterator endVertices() const;
  Set<Int>::const_iterator beginNeighbors(Int v) const;
  Set<Int>::const_iterator endNeighbors(Int v) const;
  void removeVertex(Int v); // also removes edges from/to v
  void fillInEdges(Int v); // does not remove v
  Int countFillInEdges(Int v) const;
  Int getMinfillVertex() const;
};
