#include <iostream>
#include "graph.hpp"
#include "aco.hpp"
#include "map.hpp"
#include "visualizer.hpp"
#include <vector>
#include <string>
#include <chrono>
#include "utils.hpp"

#define SHOWCASE_EXAMPLE 3

namespace
{
// 'S' is start, 'E' is end, '#' is an obstacle, '.' is walkable.
#if SHOWCASE_EXAMPLE == 0
    const std::vector<std::string> DEFAULT_MAP =
    {
    "S.........",
    ".#######..",
    ".#.....#..",
    ".#.##..#..",
    "...#E..#..",
    ".#.###.#..",
    ".#.....#..",
    ".#######..",
    "..........",
    ".........." 
    };
#elif SHOWCASE_EXAMPLE == 1
    const std::vector<std::string> DEFAULT_MAP = 
    {
        "S.......#...........",
        "#######.#.#########.",
        ".......#.#.......#..",
        ".#######.#.#####.#..",
        ".#.......#.....#.#..",
        ".#.###########.#.#..",
        ".#.#...........#.#..",
        ".#.#.###########.#..",
        ".#.#.#...........#..",
        ".#.#.#.#########.#..",
        ".#...#.#.......#.#..",
        ".#####.#.#####.#.#..",
        ".......#.#...#.#.#..",
        "########.#.#.#.#.#..",
        ".........#.#.#.#.#..",
        "##########.#.#.#.#..",
        "...........#.#.#.#..",
        ".###########.#.#.#..",
        ".............#...E..",
        "...................." 
    };
#elif SHOWCASE_EXAMPLE == 2
    const std::vector<std::string> DEFAULT_MAP = 
    {
        "S..................#....................", // 0
        "##################.#.##################.", // 1
        "...................#.#..................", // 2
        ".#################.#.#.################.", // 3
        ".#.................#.#.#..............#.", // 4
        ".#.#################.#.#.############.#.", // 5
        ".#.#.................#.#.#..........#.#.", // 6
        ".#.#.#################.#.##########.#.#.", // 7
        ".#.#.#.................#.#........#.#.#.", // 8
        ".#.#.#.#################.#.######.#.#.#.", // 9
        ".#.#.#.#.................#.#....#.#.#.#.", // 10
        ".#.#.#.#.#################.#.##.#.#.#.#.", // 11
        ".#.#.#.#.#...............#.#.#..#.#.#.#.", // 12
        ".#.#.#.#.#.#############.#.#.#..#.#.#.#.", // 13
        ".#.#.#.#.#.#...........#.#.#.#..#.#.#.#.", // 14
        ".#.#.#.#.#.#.#########.#.#.#.#..#.#.#.#.", // 15
        ".#.#.#.#.#.#.#.......#.#.#.#.#..#.#.#.#.", // 16
        ".#.#.#.#.#.#.#.#####.#.#.#.#.#..#.#.#.#.", // 17
        ".#.#.#.#.#.#.#.#...#.#.#.#.#.#..#.#.#.#.", // 18
        ".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#..#.#.#.#.", // 19
        ".#.#.#.#.#.#.#.#.#.#.#.#.#.#.#..#.#.#.#.", // 20
        ".#.#.#.#.#.#.#.#...#.#.#.#.#.#..#.#.#.#.", // 21
        ".#.#.#.#.#.#.#.#####.#.#.#.#.#..#.#.#.#.", // 22
        ".#.#.#.#.#.#.#.......#.#.#.#.#..#.#.#.#.", // 23
        ".#.#.#.#.#.#.#########.#.#.#.#..#.#.#.#.", // 24
        ".#.#.#.#.#.#...........#.#.#.#..#.#.#.#.", // 25
        ".#.#.#.#.#.#############.#.#.#..#.#.#.#.", // 26
        ".#.#.#.#.#...............#.#.#..#.#.#.#.", // 27
        ".#.#.#.#.#################.#.##.#.#.#.#.", // 28
        ".#.#.#.#.................#.#....#.#.#.#.", // 29
        ".#.#.#.#################.#.######.#.#.#.", // 30
        ".#.#.#.................#.#........#.#.#.", // 31
        ".#.#.#################.#.##########.#.#.", // 32
        ".#.#.................#.#.#..........#.#.", // 33
        ".#.#################.#.#.############.#.", // 34
        ".#.................#.#.#..............#.", // 35
        ".#################.#.#.################.", // 36
        "...................#.#..................", // 37
        "##################.#.#.................E", // 38
        "........................................"  // 39
    };
#elif SHOWCASE_EXAMPLE == 3
    const std::vector<std::string> DEFAULT_MAP =
    {
        "S...............",
        "................",
        "...W............",
        "......##W#...W..",
        "......#..#......",
        "......###.......",
        "........W.......",
        "......####......",
        "......#..#......",
        "......####......",
        "...W............",
        "..............W." 
    };
#endif
}

int main()
{
#ifdef USE_CUDA
    std::cout << "Cuda is supported!" << std::endl;
#else
    std::cout << "Cuda is not supported!" << std::endl;
#endif

    Map map(DEFAULT_MAP);
    
    auto startNodeOpt = map.getStartNode();
    auto endNodeOpt = map.getEndNode();
    const bool hasTSP = !map.getWaypoints().empty();

    if (!startNodeOpt.has_value())
    {
        std::cerr << "Map must have a start ('" << Map::START << "') point." << std::endl;
        return 1;
    }
    if (!hasTSP && !endNodeOpt.has_value())
    {
        std::cerr << "Map must have an end ('" << Map::END << "') point." << std::endl;
        return 1;
    }

    Graph::Node startNode = *startNodeOpt;
    Graph::Node endNode = endNodeOpt.value_or(Graph::INVALID_NODE);
    
    ACO::Params params;
#if SHOWCASE_EXAMPLE == 2
    params.numAnts = 64;
    params.iterations = 2000;
    params.evaporation = 0.1f;
    params.minimumPheromone = 0.0001f;
#endif
    if (hasTSP)
    {
        params.numAnts = 64;
        params.iterations = 500;
        params.evaporation = 0.3f;
        params.minimumPheromone = 0.001f;
        params.depositBestOnly = true;
    }
    ACO::ACO aco(params);
    Graph graph = map.toGraph();

    if (hasTSP)
    {
        const auto& waypoints = map.getWaypoints();
        const bool classicTSP = (endNode == Graph::INVALID_NODE);
        std::cout << "Running ACO-TSP (" << (classicTSP ? "classic, return to start" : "path, fixed end")
                  << ") on map with " << waypoints.size() << " waypoints..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        auto tspResult = aco.runTSP(graph, startNode, endNode, waypoints);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::cout << "ACO-TSP completed in "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()
                  << " ms" << std::endl;

        if (utils::isSimilar(tspResult.bestCost, ACO::Path::NO_PATH_COST))
            std::cout << "No tour found." << std::endl;
        else
        {
            std::cout << "Best Tour Cost: " << tspResult.bestCost << std::endl;
            std::cout << "Visit order (grid coords): ";
            const int numKeyNodes = static_cast<int>(waypoints.size()) + (classicTSP ? 1 : 2);
            for (int idx : tspResult.bestVisitOrder)
            {
                Graph::Node node;
                if (idx == 0)                              node = startNode;
                else if (!classicTSP && idx == numKeyNodes - 1) node = endNode;
                else                                       node = waypoints[idx - 1];
                std::cout << "(" << node / map.getWidth() << "," << node % map.getWidth() << ") ";
            }
            std::cout << std::endl;
        }

        Visualizer visualizer(map, tspResult.acoResult);
        visualizer.run();
    }
    else
    {
        std::cout << "Running ACO on map..." << std::endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        auto result = aco.run(graph, startNode, endNode);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::cout << "ACO completed in " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " ms" << std::endl;

        if (utils::isSimilar(result.bestPath.cost, ACO::Path::NO_PATH_COST))
            std::cout << "No path found." << std::endl;
        else
        {
            std::cout << "Best Path Cost: " << result.bestPath.cost << std::endl;
            std::cout << "Path: ";
            for (auto node : result.bestPath.nodes)
                std::cout << "(" << node / map.getWidth() << "," << node % map.getWidth() << ") ";
            std::cout << std::endl;
        }

        Visualizer visualizer(map, result);
        visualizer.run();
    }

    return 0;
}
