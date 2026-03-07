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
    static constexpr char WAYPOINT = 'W';
    static constexpr float CARDINAL_WEIGHT = 1.0f;
    static constexpr float DIAGONAL_WEIGHT = 1.41421356237f; // sqrt(2)
    explicit Map(const std::vector<std::string>& grid);

    [[nodiscard]] Graph toGraph() const;

    [[nodiscard]] std::optional<Graph::Node> getStartNode() const { return m_startNode; }
    [[nodiscard]] std::optional<Graph::Node> getEndNode() const { return m_endNode; }
    [[nodiscard]] const std::vector<Graph::Node>& getWaypoints() const { return m_waypoints; }

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

    [[nodiscard]] char getCell(int row, int col) const;

private:
    [[nodiscard]] bool isWalkable(int row, int col) const;
    [[nodiscard]] int toNodeId(int row, int col) const;

    std::vector<std::string> m_grid;
    int m_height;
    int m_width;
    std::optional<Graph::Node> m_startNode;
    std::optional<Graph::Node> m_endNode;
    std::vector<Graph::Node> m_waypoints;

};