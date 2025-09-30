#include <graph.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Graph basic functionality", "[graph]") 
{
    Graph g(5);

    SECTION("Add edges and check neighbors") 
    {
        g.addEdge(0, 1, 10);
        g.addEdge(0, 2, 20);
        g.addEdge(1, 2, 30);

        auto neighbors0 = g.getNeighbors(0);
        REQUIRE(neighbors0.size() == 2);
        REQUIRE((neighbors0[0] == Graph::Neighbor{1, 10} || neighbors0[1] == Graph::Neighbor{1, 10}));
        REQUIRE((neighbors0[0] == Graph::Neighbor{2, 20} || neighbors0[1] == Graph::Neighbor{2, 20}));

        auto neighbors1 = g.getNeighbors(1);
        REQUIRE(neighbors1.size() == 2);
        REQUIRE((neighbors1[0] == Graph::Neighbor{0, 10} || neighbors1[1] == Graph::Neighbor{0, 10}));
        REQUIRE((neighbors1[0] == Graph::Neighbor{2, 30} || neighbors1[1] == Graph::Neighbor{2, 30}));
    }

    SECTION("Check degree of nodes") 
    {
        g.addEdge(0, 1, 10);
        g.addEdge(0, 2, 20);
        g.addEdge(1, 2, 30);

        REQUIRE(g.degree(0) == 2);
        REQUIRE(g.degree(1) == 2);
        REQUIRE(g.degree(2) == 2);
        REQUIRE(g.degree(3) == 0);
    }

    SECTION("Handle invalid node indices") 
    {
        auto neighborsInvalid = g.getNeighbors(-1);
        REQUIRE(neighborsInvalid.size() == 0);

        neighborsInvalid = g.getNeighbors(5);
        REQUIRE(neighborsInvalid.size() == 0);

        REQUIRE(g.degree(-1) == 0);
        REQUIRE(g.degree(5) == 0);
    }
}