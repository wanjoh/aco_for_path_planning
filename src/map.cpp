#include "map.hpp"
#include <cmath>

Map::Map(const std::vector<std::string>& grid)
    : m_grid(grid)
    , m_height(static_cast<int>(grid.size()))
    , m_width(m_height > 0 ? static_cast<int>(grid[0].size()) : 0)
    , m_startNode(std::nullopt)
    , m_endNode(std::nullopt)
{
    for (int x = 0; x < m_height; ++x) 
    {
        for (int y = 0; y < m_width; ++y) 
        {
            if (m_grid[x][y] == START) 
                m_startNode = toNodeId(x, y);
            else if (m_grid[x][y] == END)
                m_endNode = toNodeId(x, y);
        }
    }
    // todo : add check for multiple starts and ends; add check for non-rectangular grid
}

bool Map::isWalkable(int x, int y) const 
{
    if (x < 0 || x >= m_height || y < 0 || y >= m_width)
        return false;
    return m_grid[x][y] != OBSTACLE;
}

int Map::toNodeId(int x, int y) const 
{
    return x * m_width + y;
}

Graph Map::toGraph() const 
{
    const int numNodes = m_width * m_height;
    Graph graph(numNodes);

    static const float cardinalWeight = 1.0f;
    static const float diagonalWeight = std::sqrt(2.0f);

    for (int x = 0; x < m_height; ++x) {
        for (int y = 0; y < m_width; ++y) 
        {
            if (!isWalkable(x, y))
                continue;

            Graph::Node currentNode = toNodeId(x, y);

            // add neighbors
            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy) 
                {
                    if (dx == 0 && dy == 0)
                        continue;

                    int nx = x + dx;
                    int ny = y + dy;

                    if (isWalkable(nx, ny)) 
                    {
                        // a more advanced implementation might prevent diagonal movement
                        // if the path is "squeezing" between two obstacles; for now it's allowed.
                        Graph::Node neighborNode = toNodeId(nx, ny);
                        bool isDiagonal = (dx != 0 && dy != 0);
                        float weight = isDiagonal ? diagonalWeight : cardinalWeight;
                        
                        graph.addDirectedEdge(currentNode, neighborNode, weight);
                    }
                }
            }
        }
    }

    graph.finalize();
    return graph;
}