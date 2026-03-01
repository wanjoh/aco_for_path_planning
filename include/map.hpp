#pragma once

#include "graph.hpp"
#include <vector>
#include <string>
#include <optional>

class Map
{
public:
    static constexpr char OBSTACLE = '#';
    static constexpr char START = 'S';
    static constexpr char END = 'E';
    explicit Map(const std::vector<std::string>& grid);

    [[nodiscard]] Graph toGraph() const;

    [[nodiscard]] std::optional<Graph::Node> getStartNode() const { return m_startNode; }
    [[nodiscard]] std::optional<Graph::Node> getEndNode() const { return m_endNode; }

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

private:
    [[nodiscard]] bool isWalkable(int x, int y) const;
    [[nodiscard]] int toNodeId(int x, int y) const;

    std::vector<std::string> m_grid;
    int m_height;
    int m_width;
    std::optional<Graph::Node> m_startNode;
    std::optional<Graph::Node> m_endNode;

};