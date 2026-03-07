#include "map.hpp"

Map::Map(const std::vector<std::string>& grid)
    : m_grid(grid)
    , m_height(static_cast<int>(grid.size()))
    , m_width(m_height > 0 ? static_cast<int>(grid[0].size()) : 0)
    , m_startNode(std::nullopt)
    , m_endNode(std::nullopt)
{
    for (int row = 0; row < m_height; ++row) 
    {
        for (int col = 0; col < m_width; ++col) 
        {
            if (m_grid[row][col] == START)
                m_startNode = toNodeId(row, col);
            else if (m_grid[row][col] == END)
                m_endNode = toNodeId(row, col);
            else if (m_grid[row][col] == WAYPOINT)
                m_waypoints.push_back(toNodeId(row, col));
        }
    }
    // todo : add check for multiple starts and ends; add check for non-rectangular grid
}

bool Map::isWalkable(int row, int col) const 
{
    if (row < 0 || row >= m_height || col < 0 || col >= m_width)
        return false;
    return m_grid[row][col] != OBSTACLE;
}

char Map::getCell(int row, int col) const
{
    if (row < 0 || row >= m_height || col < 0 || col >= m_width)
        return OBSTACLE;
    return m_grid[row][col];
}

int Map::toNodeId(int row, int col) const 
{
    return row * m_width + col;
}

Graph Map::toGraph() const 
{
    const int numNodes = m_width * m_height;
    Graph graph(numNodes);

    for (int row = 0; row < m_height; ++row) {
        for (int col = 0; col < m_width; ++col) 
        {
            if (!isWalkable(row, col))
                continue;

            Graph::Node currentNode = toNodeId(row, col);

            // add neighbors
            for (int dRow = -1; dRow <= 1; ++dRow)
            {
                for (int dCol = -1; dCol <= 1; ++dCol) 
                {
                    if (dRow == 0 && dCol == 0)
                        continue;

                    int nRow = row + dRow;
                    int nCol = col + dCol;

                    if (isWalkable(nRow, nCol)) 
                    {
                        // a more advanced implementation might prevent diagonal movement
                        // if the path is "squeezing" between two obstacles; for now it's allowed.
                        Graph::Node neighborNode = toNodeId(nRow, nCol);
                        bool isDiagonal = (dRow != 0 && dCol != 0);
                        float weight = isDiagonal ? DIAGONAL_WEIGHT : CARDINAL_WEIGHT;
                        
                        graph.addDirectedEdge(currentNode, neighborNode, weight);
                    }
                }
            }
        }
    }

    graph.finalize();
    return graph;
}