#include <iostream>
#include "graph.hpp"
#include "aco.hpp"

int main() 
{
#ifdef USE_CUDA
    std::cout << "Cuda is supported!" << std::endl;
#else
    std::cout << "Cuda is not supported!" << std::endl;
#endif

    // simple graph
    // todo: create more complex graph and add a test for it
    // 0 --1--> 1 --1--> 3
    // |                 ^
    // \--2--> 2 --2-----|
    // Optimal path: 0 -> 1 -> 3 (Cost 2.0)
    // Suboptimal:   0 -> 2 -> 3 (Cost 4.0)
    
    Graph graph(4);
    graph.addEdge(0, 1, 1.0f);
    graph.addEdge(0, 2, 2.0f);
    graph.addEdge(1, 3, 1.0f);
    graph.addEdge(2, 3, 2.0f);
    graph.finalize();

    ACO::Params params;
    params.numAnts = 10;
    params.iterations = 20;
    params.alpha = 1.0f;
    params.beta = 2.0f;
    params.evaporation = 0.1f;

    ACO::ACO aco(params);
    
    std::cout << "Running ACO..." << std::endl;
    auto result = aco.run(graph, 0, 3);

    std::cout << "Best Path Cost: " << result.bestPath.cost << std::endl;
    std::cout << "Path: ";
    for (auto node : result.bestPath.nodes) {
        std::cout << node << " ";
    }
    std::cout << std::endl;

    return 0;
}
