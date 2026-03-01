#include <map.hpp>
#include <catch2/catch_test_macros.hpp>
#include "utils.hpp"

TEST_CASE("Map to Graph conversion", "[map]") 
{
    std::vector<std::string> grid = 
    {
        "S#.",
        "..#",
        "#.E"
    };

    Map map(grid);
    Graph graph = map.toGraph();

    auto startNodeOpt = map.getStartNode();
    auto endNodeOpt = map.getEndNode();

    REQUIRE(startNodeOpt.has_value());
    REQUIRE(endNodeOpt.has_value());

    Graph::Node startNode = *startNodeOpt;
    Graph::Node endNode = *endNodeOpt;

    // start and end node
    REQUIRE(startNode == 0); // (0,0)
    REQUIRE(endNode == 8);   // (2,2)

    // neighbors of the start node
    auto neighborsStart = graph.getNeighbors(startNode);
    REQUIRE(neighborsStart.size() == 2);
    REQUIRE(neighborsStart[0].first == 3); // (1,0)
    REQUIRE(utils::isSimilar(neighborsStart[0].second, Map::CARDINAL_WEIGHT));
    REQUIRE(neighborsStart[1].first == 4); // (1,1)
    REQUIRE(utils::isSimilar(neighborsStart[1].second, Map::DIAGONAL_WEIGHT));

    // neighbors of the end node
    auto neighborsEnd = graph.getNeighbors(endNode);
    REQUIRE(neighborsEnd.size() == 2);
    REQUIRE(neighborsEnd[0].first == 4); // (1,1)
    REQUIRE(utils::isSimilar(neighborsEnd[0].second, Map::DIAGONAL_WEIGHT));
    REQUIRE(neighborsEnd[1].first == 7); // (2,1)
    REQUIRE(utils::isSimilar(neighborsEnd[1].second, Map::CARDINAL_WEIGHT));

    // obstacles should not have neighbors in the graph representation
    auto neighborsObstacle = graph.getNeighbors(1); // (0,1) is an obstacle
    REQUIRE(neighborsObstacle.empty());
}