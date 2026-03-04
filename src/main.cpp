#include <iostream>
#include "graph.hpp"
#include "aco.hpp"
#include "map.hpp"
#include "visualizer.hpp"
#include <vector>
#include <string>
#include <chrono>
#include "utils.hpp"

#define SHOWCASE_EXAMPLE 2

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
    
    if (!startNodeOpt.has_value() || !endNodeOpt.has_value()) 
    {
        std::cerr << "Map must have a start ('" << Map::START << "') and an end ('" << Map::END << "') point." << std::endl;
        return 1;
    }
    
    Graph::Node startNode = *startNodeOpt;
    Graph::Node endNode = *endNodeOpt;
    
    ACO::Params params;
#if SHOWCASE_EXAMPLE == 2
    params.numAnts = 64;
    params.iterations = 2000;
    params.depositBestOnly = true;
    params.evaporation = 0.1f;
    params.minimumPheromone = 0.0001f;
#endif
    ACO::ACO aco(params);
    Graph graph = map.toGraph();
    
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
        {
            int x = node / map.getWidth();
            int y = node % map.getWidth();
            std::cout << "(" << x << "," << y << ") ";
        }
        std::cout << std::endl;
    }

    Visualizer visualizer(map, result);
    visualizer.run();

    return 0;
}
