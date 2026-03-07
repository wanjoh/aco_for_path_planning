#include "graph.hpp"

#include <algorithm>
#include <cassert>
#include <queue>


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

std::pair<std::vector<Graph::Weight>, std::vector<Graph::Node>>
dijkstraWithPrev(const Graph& graph, Graph::Node source)
{
    const int n = graph.getNumNodes();
    constexpr Graph::Weight INF = std::numeric_limits<Graph::Weight>::max();
    std::vector<Graph::Weight> dist(n, INF);
    std::vector<Graph::Node> prev(n, Graph::INVALID_NODE);
    dist[source] = 0.0f;

    using Entry = std::pair<Graph::Weight, Graph::Node>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
    pq.push({0.0f, source});

    while (!pq.empty())
    {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (const auto& [v, w] : graph.getNeighbors(u))
        {
            float nd = dist[u] + w;
            if (nd < dist[v])
            {
                dist[v] = nd;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }
    return {dist, prev};
}