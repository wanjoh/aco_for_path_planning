#pragma once
#include "graph.hpp"
#include <memory>

namespace ACO
{
    struct Params
    {
        int numAnts          = 64;
        int iterations       = 100;
        float alpha          = 1.0f;  // pheromone weight
        float beta           = 2.0f;  // heuristic weight
        float evaporation    = 0.5f;
        uint32_t seed        = 919324;
        bool depositBestOnly = true;
        float minimumPheromone = 0.01f;
    };

    struct Path
    {
        static constexpr Graph::Weight NO_PATH_COST = std::numeric_limits<Graph::Weight>::max();
        std::vector<Graph::Node> nodes;
        Graph::Weight cost = NO_PATH_COST;
    };

    struct Result
    {
        Path bestPath;
        std::vector<Path> pathsPerIteration;
    };

    class ACO
    {
    public:
        explicit ACO(ACO::Params params) noexcept : m_params(std::move(params)) {}

        [[nodiscard]] Result run(const Graph& graph, Graph::Node start, Graph::Node end) noexcept;
        [[nodiscard]] const Params& getParams() const noexcept { return m_params; }

    private:
        Params m_params;
    };

}