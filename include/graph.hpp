#pragma once

#include <vector>
#include <utility>
#include <span>

class Graph
{
public:
    using Node = int;
    constexpr static Node INVALID_NODE = -1;
    using Weight = float;
    using Neighbor = std::pair<Node, Weight>;
    explicit Graph(int numNodes);

    void addDirectedEdge(Node from, Node to, Weight weight) noexcept;
    void addEdge(Node nodeU, Node nodeV, Weight weight) noexcept;
    
    [[nodiscard]] std::span<const Neighbor> getNeighbors(Node n) const noexcept;
    [[nodiscard]] int getNumNodes() const noexcept { return static_cast<int>(m_adjacencyList.size()); }
    [[nodiscard]] int degree(Node n) const noexcept;

    // Prevents further modifications to the graph
    void finalize() noexcept { m_sealed = true; }

    // compressed sparce row format for GPU
    struct CSR
    {
        std::vector<int> row_offsets; // size = numNodes + 1
        std::vector<Node> col_indices; // size = numEdges
        std::vector<Weight> weights; // size = numEdges
    };
    [[nodiscard]] CSR toCSR() const;
    

private:
    std::vector<std::vector<Neighbor>> m_adjacencyList;  // todo: this can be vector<set> ?
    bool m_sealed;
};