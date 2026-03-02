#pragma once

#include "graph.hpp"
#include "aco.hpp"


namespace ACO 
{
    Result run_cuda(const Graph& graph, Graph::Node start, Graph::Node end, const Params& params);
}
