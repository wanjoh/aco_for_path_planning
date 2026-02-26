#include "aco.hpp"
#include <random>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iostream>

namespace ACO
{
    Result ACO::run(const Graph& graph, Graph::Node start, Graph::Node end) noexcept
    {
        Result result;
        const int numNodes = graph.getNumNodes();
        
        std::vector<std::vector<float>> pheromones(numNodes, std::vector<float>(numNodes, 0.1f));

        std::mt19937 rng(m_params.seed);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        result.bestPath.cost = std::numeric_limits<Graph::Weight>::max();

        for (int iter = 0; iter < m_params.iterations; ++iter)
        {
            std::vector<Path> antPaths;
            antPaths.reserve(m_params.numAnts);

            for (int k = 0; k < m_params.numAnts; ++k)
            {
                Path currentPath;
                currentPath.nodes.push_back(start);
                currentPath.cost = 0;
                
                std::vector<bool> visited(numNodes, false);
                visited[start] = true;
                
                Graph::Node currentNode = start;
                bool stuck = false;

                while (currentNode != end)
                {
                    auto neighbors = graph.getNeighbors(currentNode);
                    std::vector<Graph::Node> candidates;
                    std::vector<float> probabilities;
                    float probSum = 0.0f;

                    // probability calculation for each valid neighbor
                    for (const auto& [neighbor, weight] : neighbors)
                    {
                        if (!visited[neighbor])
                        {
                            candidates.push_back(neighbor);
                            float tau = pheromones[currentNode][neighbor];
                            float eta = 1.0f / (std::max(weight, 1e-6f));
                            
                            float prob = std::pow(tau, m_params.alpha) * std::pow(eta, m_params.beta);
                            probabilities.push_back(prob);
                            probSum += prob;
                        }
                    }

                    if (candidates.empty())
                    {
                        stuck = true;
                        break;
                    }

                    // roulette wheel selection
                    float r = dist(rng) * probSum;
                    float runningSum = 0.0f;
                    Graph::Node nextNode = Graph::INVALID_NODE;
                    Graph::Weight edgeWeight = 0;

                    for (size_t i = 0; i < candidates.size(); ++i)
                    {
                        runningSum += probabilities[i];
                        if (runningSum >= r)
                        {
                            nextNode = candidates[i];
                            break;
                        }
                    }
                    
                    // handling float precision issues
                    if (nextNode == Graph::INVALID_NODE) nextNode = candidates.back();

                   
                    for(const auto& [n, w] : neighbors)
                    { 
                        if(n == nextNode)
                        {
                            edgeWeight = w; 
                            break; 
                        }
                    }

                    visited[nextNode] = true;
                    currentPath.nodes.push_back(nextNode);
                    currentPath.cost += edgeWeight;
                    currentNode = nextNode;
                }

                if (!stuck)
                {
                    antPaths.push_back(std::move(currentPath));
                }
            }

            // evaporation
            for (auto& row : pheromones)
            {
                for (auto& p : row) {
                    p *= (1.0f - m_params.evaporation);
                    p = std::max(p, m_params.minimumPheromone);
                }
            }

            // update global best path
            for (const auto& path : antPaths)
            {
                if (path.cost < result.bestPath.cost)
                {
                    result.bestPath = path;
                }
            }
            
            // record history
            if (result.bestPath.cost != std::numeric_limits<Graph::Weight>::max())
                result.costPerIteration.push_back(result.bestPath.cost);
            else 
                result.costPerIteration.push_back(Result::NO_PATH_COST);

            // eeposit pheromones
            if (m_params.depositBestOnly)
            {
                if (result.bestPath.nodes.size() > 1)
                {
                    float delta = 1.0f / result.bestPath.cost;
                    for (size_t i = 0; i < result.bestPath.nodes.size() - 1; ++i)
                    {
                        Graph::Node u = result.bestPath.nodes[i];
                        Graph::Node v = result.bestPath.nodes[i+1];
                        pheromones[u][v] += delta;
                    }
                }
            }
            else
            {
                for (const auto& path : antPaths)
                {
                    float delta = 1.0f / path.cost;
                    for (size_t i = 0; i < path.nodes.size() - 1; ++i)
                    {
                        Graph::Node u = path.nodes[i];
                        Graph::Node v = path.nodes[i+1];
                        pheromones[u][v] += delta;
                    }
                }
            }
        }

        return result;
    }
}