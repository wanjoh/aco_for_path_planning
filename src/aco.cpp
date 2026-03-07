#include "aco.hpp"
#include <random>
#include <limits>
#include <algorithm>
#include <cmath>


namespace ACO
{
Result ACO::run(const Graph& graph, Graph::Node start, Graph::Node end) noexcept
{
    Result result;
    const int numNodes = graph.getNumNodes();
    
    std::vector<std::vector<float>> pheromones(numNodes, std::vector<float>(numNodes, m_params.minimumPheromone));

    std::mt19937 rng(m_params.seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    result.bestPath.cost = Path::NO_PATH_COST;

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

        // get best path for this iteration
        Path currentBest;
        for (const auto& path : antPaths)
        {
            if (path.cost < currentBest.cost)
            {
                currentBest = path;
            }
        }
        
        result.pathsPerIteration.push_back(currentBest);
        result.bestPath = currentBest.cost < result.bestPath.cost ? currentBest : result.bestPath;

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
// helper function to reconstruct a grid path from Dijkstra's predecessor array
static std::vector<Graph::Node> reconstructGridPath(const std::vector<Graph::Node>& prev,
                                                     Graph::Node from, Graph::Node to)
{
    std::vector<Graph::Node> path;
    for (Graph::Node n = to; n != Graph::INVALID_NODE; n = prev[n])
        path.push_back(n);
    std::reverse(path.begin(), path.end());
    if (path.empty() || path.front() != from)
        return {}; // unreachable
    return path;
}

TSPResult ACO::runTSP(const Graph& graph, Graph::Node start, Graph::Node end,
                      const std::vector<Graph::Node>& waypoints) noexcept
{
    std::vector<Graph::Node> keyNodes;
    keyNodes.push_back(start);
    for (auto w : waypoints) keyNodes.push_back(w);
    keyNodes.push_back(end);
    const int K = static_cast<int>(keyNodes.size());
    const int numWaypoints = K - 2;

    constexpr Graph::Weight INF = Path::NO_PATH_COST;
    std::vector<std::vector<Graph::Weight>> distMatrix(K, std::vector<Graph::Weight>(K, INF));
    std::vector<std::vector<std::vector<Graph::Node>>> pathMatrix(K, std::vector<std::vector<Graph::Node>>(K));

    for (int i = 0; i < K; ++i)
    {
        auto [dist, prev] = dijkstraWithPrev(graph, keyNodes[i]);
        for (int j = 0; j < K; ++j)
        {
            if (i == j) { distMatrix[i][j] = 0.0f; continue; }
            distMatrix[i][j] = dist[keyNodes[j]];
            if (dist[keyNodes[j]] < INF)
                pathMatrix[i][j] = reconstructGridPath(prev, keyNodes[i], keyNodes[j]);
        }
    }

    std::vector<std::vector<float>> pheromones(K, std::vector<float>(K, m_params.minimumPheromone));

    std::mt19937 rng(m_params.seed);
    std::uniform_real_distribution<float> dist_rng(0.0f, 1.0f);

    TSPResult result;
    result.acoResult.bestPath.cost = INF;

    for (int iter = 0; iter < m_params.iterations; ++iter)
    {
        std::vector<std::vector<int>> antTours;
        std::vector<Graph::Weight> antCosts;

        for (int k = 0; k < m_params.numAnts; ++k)
        {
            std::vector<int> tour;
            tour.push_back(0); // always start at index 0
            std::vector<bool> visited(K, false);
            visited[0] = true;
            visited[K - 1] = true; // reserve end; ants pick it last

            int current = 0;
            bool stuck = false;

            for (int step = 0; step < numWaypoints; ++step)
            {
                struct Candidate { int idx; float prob; };
                std::vector<Candidate> candidates;
                float probSum = 0.0f;

                for (int j = 1; j <= numWaypoints; ++j)
                {
                    if (!visited[j] && distMatrix[current][j] < INF)
                    {
                        float tau = pheromones[current][j];
                        float eta = 1.0f / std::max(distMatrix[current][j], 1e-6f);
                        float prob = std::pow(tau, m_params.alpha) * std::pow(eta, m_params.beta);
                        candidates.push_back({j, prob});
                        probSum += prob;
                    }
                }

                if (candidates.empty()) { stuck = true; break; }

                float r = dist_rng(rng) * probSum;
                float running = 0.0f;
                int selected = candidates.back().idx;
                for (const auto& c : candidates)
                {
                    running += c.prob;
                    if (running >= r) { selected = c.idx; break; }
                }
                visited[selected] = true;
                tour.push_back(selected);
                current = selected;
            }

            if (!stuck && distMatrix[current][K - 1] < INF)
            {
                tour.push_back(K - 1);
                Graph::Weight tourCost = 0.0f;
                bool valid = true;
                for (int t = 0; t < static_cast<int>(tour.size()) - 1; ++t)
                {
                    Graph::Weight leg = distMatrix[tour[t]][tour[t + 1]];
                    if (leg >= INF) { valid = false; break; }
                    tourCost += leg;
                }
                if (valid) { antTours.push_back(tour); antCosts.push_back(tourCost); }
            }
        }

        int bestIdx = -1;
        Graph::Weight bestIterCost = INF;
        for (int k = 0; k < static_cast<int>(antCosts.size()); ++k)
        {
            if (antCosts[k] < bestIterCost) { bestIterCost = antCosts[k]; bestIdx = k; }
        }

        Path iterPath;
        iterPath.cost = bestIterCost;
        if (bestIdx >= 0)
        {
            const auto& tour = antTours[bestIdx];
            for (int t = 0; t < static_cast<int>(tour.size()) - 1; ++t)
            {
                const auto& leg = pathMatrix[tour[t]][tour[t + 1]];
                if (t == 0)
                    for (auto n : leg) iterPath.nodes.push_back(n);
                else
                    for (size_t ni = 1; ni < leg.size(); ++ni) iterPath.nodes.push_back(leg[ni]);
            }
        }
        result.acoResult.pathsPerIteration.push_back(iterPath);

        if (bestIdx >= 0 && bestIterCost < result.bestCost)
        {
            result.bestCost = bestIterCost;
            result.bestVisitOrder = antTours[bestIdx];
            result.acoResult.bestPath = iterPath;
        }

        for (auto& row : pheromones)
            for (auto& p : row) { p *= (1.0f - m_params.evaporation); p = std::max(p, m_params.minimumPheromone); }

        if (m_params.depositBestOnly)
        {
            if (result.bestCost < INF)
            {
                float delta = 1.0f / result.bestCost;
                for (int t = 0; t < static_cast<int>(result.bestVisitOrder.size()) - 1; ++t)
                    pheromones[result.bestVisitOrder[t]][result.bestVisitOrder[t + 1]] += delta;
            }
        }
        else
        {
            for (int k = 0; k < static_cast<int>(antTours.size()); ++k)
            {
                float delta = 1.0f / antCosts[k];
                for (int t = 0; t < static_cast<int>(antTours[k].size()) - 1; ++t)
                    pheromones[antTours[k][t]][antTours[k][t + 1]] += delta;
            }
        }
    }

    return result;
}

} // namespace ACO