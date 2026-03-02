#include "aco.hpp"
#include <random>
#include <limits>
#include <algorithm>


namespace ACO
{
Result ACO::run(const Graph& graph, Graph::Node start, Graph::Node end) noexcept
{
    Result result;
    const int numNodes = graph.getNumNodes();
    
    std::vector<std::vector<float>> pheromones(numNodes, std::vector<float>(numNodes, 0.1f));

    std::mt19937 rng(m_params.seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    result.bestPath.cost = Result::NO_PATH_COST;

    for (int iter = 0; iter < m_params.iterations; ++iter)
    {
        std::vector<Path> antPaths;
        antPaths.reserve(NUM_ANTS);

        for (int k = 0; k < NUM_ANTS; ++k)
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

                struct MoveCandidate {
                    Graph::Node to;
                    Graph::Weight weight;
                    float probability;
                };
                std::vector<MoveCandidate> candidates;
                float probSum = 0.0f;

                // probability calculation for each valid neighbor
                for (const auto& [neighbor, weight] : neighbors)
                {
                    if (!visited[neighbor])
                    {
                        float tau = pheromones[currentNode][neighbor];
                        float eta = 1.0f / (std::max(weight, 1e-6f));
                        
                        float prob = std::pow(tau, m_params.alpha) * std::pow(eta, m_params.beta);
                        candidates.push_back({neighbor, weight, prob});
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
                MoveCandidate selected_candidate = candidates.back();  // handlign precision issues

                for (size_t i = 0; i < candidates.size(); ++i)
                {
                    runningSum += candidates[i].probability;
                    if (runningSum >= r)
                    {
                        selected_candidate = candidates[i];
                        break;
                    }
                }

                visited[selected_candidate.to] = true;
                currentPath.nodes.push_back(selected_candidate.to);
                currentPath.cost += selected_candidate.weight;
                currentNode = selected_candidate.to;
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
        result.costPerIteration.push_back(result.bestPath.cost);

        // deposit pheromones
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
} // namespace ACO