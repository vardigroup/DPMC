/* inclusions *****************************************************************/

#include "../interface/graph.hpp"

/* classes ********************************************************************/

/* class Graph ****************************************************************/

bool Graph::isNeighbor(Int v1, Int v2) const {
  return util::isFound(v1, adjacencyMap.at(v2));
}

bool Graph::hasPath(Int from, Int to, Set<Int> &visitedVertices) const {
  if (from == to) return true;

  visitedVertices.insert(from);

  Set<Int> unvisitedNeighbors;
  util::differ(unvisitedNeighbors, adjacencyMap.at(from), visitedVertices);

  for (Int v : unvisitedNeighbors) {
    if (hasPath(v, to, visitedVertices)) {
      return true;
    }
  }

  return false;
}

bool Graph::hasPath(Int from, Int to) const {
  Set<Int> visitedVertices;
  return hasPath(from, to, visitedVertices);
}

void Graph::printVertices() const {
  cout << "vertices: ";
  for (Int vertex : vertices) {
    cout << vertex << " ";
  }
  cout << "\n\n";
}

void Graph::printAdjacencyMap() const {
  cout << "adjacency map {\n";
  for (auto pair = adjacencyMap.begin(); pair != adjacencyMap.end(); pair++) {
    auto vertex = pair->first;
    auto neighbors = pair->second;
    cout << "\t" << vertex << " : ";
    for (auto neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++) {
      cout << *neighbor << " ";
    }
    cout << "\n";
  }
  cout << "}\n\n";
}

Graph::Graph(const Set<Int> &vs) {
  vertices = vs;
  for (Int v : vs) {
    adjacencyMap[v] = Set<Int>();
  }
}

void Graph::addEdge(Int v1, Int v2) {
  adjacencyMap.at(v1).insert(v2);
  adjacencyMap.at(v2).insert(v1);
}

Set<Int>::const_iterator Graph::beginVertices() const {
  return vertices.begin();
}

Set<Int>::const_iterator Graph::endVertices() const {
  return vertices.end();
}

Set<Int>::const_iterator Graph::beginNeighbors(Int v) const {
  return adjacencyMap.at(v).begin();
}

Set<Int>::const_iterator Graph::endNeighbors(Int v) const {
  return adjacencyMap.at(v).end();
}

void Graph::removeVertex(Int v) {
  vertices.erase(v);

  adjacencyMap.erase(v); // edges from v

  for (std::pair<const Int, Set<Int>> &vertexAndNeighbors : adjacencyMap) {
    vertexAndNeighbors.second.erase(v); // edges to v
  }
}

void Graph::fillInEdges(Int v) {
  for (auto neighborIt1 = beginNeighbors(v); neighborIt1 != endNeighbors(v); neighborIt1++) {
    for (auto neighborIt2 = std::next(neighborIt1); neighborIt2 != endNeighbors(v); neighborIt2++) {
      addEdge(*neighborIt1, *neighborIt2);
    }
  }
}

Int Graph::countFillInEdges(Int v) const {
  Int count = 0;
  for (auto neighborIt1 = beginNeighbors(v); neighborIt1 != endNeighbors(v); neighborIt1++) {
    for (auto neighborIt2 = std::next(neighborIt1); neighborIt2 != endNeighbors(v); neighborIt2++) {
      if (!isNeighbor(*neighborIt1, *neighborIt2)) {
        count++;
      }
    }
  }
  return count;
}

Int Graph::getMinFillVertex(const Set<Int> &unmarkedVertices) const {
  Int vertex = DUMMY_MIN_INT;
  Int fillInEdgeCount = DUMMY_MAX_INT;

  for (Int v : unmarkedVertices) {
    if (!util::isFound(v, vertices)) util::showError("vertex " + to_string(v) + " not in graph");

    Int count = countFillInEdges(v);
    if (count < fillInEdgeCount) {
      fillInEdgeCount = count;
      vertex = v;
    }
  }

  if (vertex == DUMMY_MIN_INT) util::showError("no unmarked vertex");

  return vertex;
}
