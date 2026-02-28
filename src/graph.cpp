#include "graph.hpp"

#include <algorithm>
#include <cassert>


Graph::Graph(int numNodes) 
    : m_adjacencyList(numNodes)
    , m_sealed(false) 
{
}

namespace 
{
    void insert_unique(std::vector<Graph::Neighbor>& vec, Graph::Node to, Graph::Weight w) 
    {
        if (std::none_of(vec.begin(), vec.end(), [&](const Graph::Neighbor& n) { return n.first == to; }))
        {
            vec.emplace_back(to, w);
        }
    }
} // anonymous namespace

void Graph::addDirectedEdge(Node from, Node to, Weight weight) noexcept
{
    assert( from >= 0 && from < getNumNodes() && 
            to >= 0 && to < getNumNodes() &&
            "Node indices must be within valid range.");
    assert(!m_sealed && "Cannot add edges to a sealed graph.");
    insert_unique(m_adjacencyList[from], to, weight);
}

void Graph::addEdge(Node nodeU, Node nodeV, Weight weight) noexcept
{
    addDirectedEdge(nodeU, nodeV, weight);
    addDirectedEdge(nodeV, nodeU, weight);
}

std::span<const Graph::Neighbor> Graph::getNeighbors(Node node) const noexcept
{
    if (node < 0 || node >= static_cast<int>(m_adjacencyList.size()))
    {
        return {};
    }

    const auto& v = m_adjacencyList[node];
    return {v.data(), v.size()};
}

int Graph::degree(Node n) const noexcept
{
    return static_cast<int>(getNeighbors(n).size());
}