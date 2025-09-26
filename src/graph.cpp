#include "graph.hpp"

#include <algorithm>
#include <cassert>


Graph::Graph(int numNodes) 
    : m_adjacencyList(numNodes)
    , m_sealed(false) 
{
}

void Graph::addEdge(Node nodeU, Node nodeV, Weight weight) 
{
    assert( nodeU >= 0 && nodeU < getNumNodes() && 
            nodeV >= 0 && nodeV < getNumNodes() &&
            "Node indices must be within valid range.");
    assert(!m_sealed && "Cannot add edges to a sealed graph.");

    auto insert_unique = [](std::vector<Neighbor>& vec, Node to, Weight w) 
    {
        if (std::none_of(vec.begin(), vec.end(), [&](const Neighbor& n) {return n.first == to;}))
        {
            vec.emplace_back(to, w);
        }
    };
    insert_unique(m_adjacencyList[nodeU], nodeV, weight);
    insert_unique(m_adjacencyList[nodeV], nodeU, weight);
}

std::span<const Graph::Neighbor> Graph::getNeighbors(Node node) const 
{
    if (node < 0 || node >= static_cast<int>(m_adjacencyList.size()))
    {
        return {};
    }

    const auto& v = m_adjacencyList[node];
    return {v.data(), v.size()};
}

int Graph::degree(Node n) const 
{
    return static_cast<int>(getNeighbors(n).size());
}