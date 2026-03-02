#include <iostream>
#include "graph.hpp"
#include "aco.hpp"
#include "map.hpp"
#include "utils.hpp"

#include <vector>
#include <string>
#include <chrono>

#ifdef USE_CUDA
#include "aco_cuda.cuh"
#endif


int main()
{
#ifdef USE_CUDA
    std::cout << "Cuda is supported!" << std::endl;
#else
    std::cout << "Cuda is not supported!" << std::endl;
#endif

    // 'S' is start, 'E' is end, '#' is an obstacle, '.' is walkable.
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
    
    if (!startNodeOpt.has_value() || !endNodeOpt.has_value()) 
    {
        std::cerr << "Map must have a start ('" << Map::START << "') and an end ('" << Map::END << "') point." << std::endl;
        return 1;
    }
    
    Graph::Node startNode = *startNodeOpt;
    Graph::Node endNode = *endNodeOpt;
    
    ACO::Params params;
    params.iterations = 50;
    params.alpha = 1.0f;
    params.beta = 5.0f; // Heuristic is more important in a grid
    params.evaporation = 0.5f;
    params.seed = 1234;
    
    ACO::ACO aco(params);
    Graph graph = map.toGraph();
    
    std::cout << "Running ACO on map..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    ACO::Result result;
#ifdef USE_CUDA
    result = ACO::run_cuda(graph, startNode, endNode, params);
#else
    result = aco.run(graph, startNode, endNode);
#endif
    std::cout << "ACO completed in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << " ms" << std::endl;

    if (utils::isSimilar(result.bestPath.cost, ACO::Result::NO_PATH_COST))
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

    return 0;
}
