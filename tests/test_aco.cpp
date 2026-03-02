#include "aco.hpp"
#include "map.hpp"
#include "utils.hpp"
#include <catch2/catch_test_macros.hpp>


TEST_CASE("ACO basic functionality", "[aco]")
{
    std::vector<std::string> grid = 
    {
        "S.##",
        ".#.#",
        ".#..",
        "###E"
    };

    Map map(grid);
    auto startNodeOpt = map.getStartNode();
    auto endNodeOpt = map.getEndNode();

    REQUIRE(startNodeOpt.has_value());
    REQUIRE(endNodeOpt.has_value());

    Graph::Node startNode = *startNodeOpt;
    Graph::Node endNode = *endNodeOpt;
    
    ACO::Params params;
    params.iterations = 50;
    params.alpha = 1.0f;
    params.beta = 5.0f;
    params.evaporation = 0.5f;
    params.seed = 1234;

    ACO::ACO aco(params);
    Graph graph = map.toGraph();
    auto result = aco.run(graph, startNode, endNode);

    REQUIRE(utils::isSimilar(result.bestPath.cost, 4.82843f));

}
