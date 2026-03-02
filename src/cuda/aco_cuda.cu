#include "aco_cuda.cuh"
#include "aco.hpp"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <device_launch_parameters.h>
#include <iostream>
#include <vector>
#include <array>

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "CUDA error at " << __FILE__ << ":" << __LINE__ << " code=" << err << " \"" << cudaGetErrorString(err) << "\"" << std::endl; \
            exit(1); \
        } \
    } while (0)


constexpr int THREADS_PER_BLOCK = 256;

namespace ACO 
{
__global__ void init_rng_kernel(curandState* states, unsigned long seed, int num_ants) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < num_ants) {
        curand_init(seed, idx, 0, &states[idx]);
    }
}

__global__ void construct_paths_kernel(
    const int* row_offsets, const int* col_indices, const float* weights, const int num_nodes,
    const float alpha, const float beta,
    const float* pheromones,
    curandState* rng_states,
    const int start_node, const int end_node,
    bool* visited_nodes, // flattened 2D array [ant_idx * num_nodes + node_idx]
    int* paths,          // flattened 2D array [ant_idx * max_path_len + path_step]
    float* path_costs,
    int* path_lengths,
    const int max_path_len, const int num_ants)
{
    int ant_idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (ant_idx >= num_ants) return;

    bool* visited = visited_nodes + ant_idx * num_nodes;
    int* path = paths + ant_idx * max_path_len;

    for (int i = 0; i < num_nodes; ++i)
    {
        visited[i] = false;
    }

    int current_node = start_node;
    path[0] = start_node;
    path_costs[ant_idx] = 0.0f;
    path_lengths[ant_idx] = 1;
    visited[start_node] = true;

    while (current_node != end_node && path_lengths[ant_idx] < max_path_len)
    {
        int edge_start_idx = row_offsets[current_node];
        int edge_end_idx = row_offsets[current_node + 1];

        float prob_sum = 0.0f;

        for (int i = edge_start_idx; i < edge_end_idx; ++i)
        {
            int neighbor_node = col_indices[i];
            if (!visited[neighbor_node])
            {
                float pheromone = pheromones[current_node * num_nodes + neighbor_node];
                float heuristic = 1.0f / fmaxf(weights[i], 1e-6f);
                prob_sum += powf(pheromone, alpha) * powf(heuristic, beta);
            }
        }

        if (prob_sum == 0.0f)
        {
            path_lengths[ant_idx] = -1;
            return;
        }

        float r = curand_uniform(&rng_states[ant_idx]) * prob_sum;
        float running_sum = 0.0f;
        int next_node = -1;
        float edge_weight = 0.0f;

        for (int i = edge_start_idx; i < edge_end_idx; ++i)
        {
            int neighbor_node = col_indices[i];
            if (!visited[neighbor_node])
            {
                float pheromone = pheromones[current_node * num_nodes + neighbor_node];
                float heuristic = 1.0f / fmaxf(weights[i], 1e-6f);
                running_sum += powf(pheromone, alpha) * powf(heuristic, beta);
                if (running_sum >= r)
                {
                    next_node = neighbor_node;
                    edge_weight = weights[i];
                    break;
                }
            }
        }

        if (next_node == -1)
        {
            path_lengths[ant_idx] = -1;
            return;
        }

        current_node = next_node;
        visited[current_node] = true;
        path_costs[ant_idx] += edge_weight;
        path[path_lengths[ant_idx]] = current_node;
        path_lengths[ant_idx]++;
    }

    if (current_node != end_node) {
        path_lengths[ant_idx] = -1;
    }
}

__global__ void evaporation_kernel(float* pheromones, int matrix_size, float evaporation_rate, float min_pheromone) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < matrix_size) {
        pheromones[idx] *= (1.0f - evaporation_rate);
        pheromones[idx] = fmaxf(pheromones[idx], min_pheromone);
    }
}

__global__ void deposit_kernel(
    float* pheromones,
    const int* paths, const float* path_costs, const int* path_lengths,
    const int* best_path, const float best_path_cost, const int best_path_length,
    const bool deposit_best_only, const int max_path_len, const int num_nodes, const int num_ants)
{
    if (deposit_best_only)
    {
        if (threadIdx.x == 0 && blockIdx.x == 0)
        {
            if (best_path_length <= 1) return;
            float delta = 1.0f / best_path_cost;
            for (int i = 0; i < best_path_length - 1; ++i)
            {
                int u = best_path[i];
                int v = best_path[i + 1];
                atomicAdd(&pheromones[u * num_nodes + v], delta);
            }
        }
    } else 
    {
        int ant_idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (ant_idx >= num_ants) return;

        if (path_lengths[ant_idx] <= 1) return; // not a valid path

        const int* ant_path = paths + ant_idx * max_path_len;
        float delta = 1.0f / path_costs[ant_idx];

        for (int i = 0; i < path_lengths[ant_idx] - 1; ++i)
        {
            int u = ant_path[i];
            int v = ant_path[i + 1];
            atomicAdd(&pheromones[u * num_nodes + v], delta);
        }
    }
}

Result run_cuda(const Graph& graph, Graph::Node start, Graph::Node end, const Params& params) {
    std::cout << "Initializing CUDA ACO..." << std::endl;
    const int num_nodes = graph.getNumNodes();
    const int max_path_len = num_nodes; // A safe upper bound for path length

    Graph::CSR csr = graph.toCSR();
    std::vector<float> h_pheromones(num_nodes * num_nodes, 0.1f);

    int *d_row_offsets, *d_col_indices;
    float *d_weights, *d_pheromones;
    curandState* d_rng_states;
    bool* d_visited_nodes;
    int *d_paths, *d_path_lengths;
    float* d_path_costs;

    CUDA_CHECK(cudaMalloc(&d_row_offsets, csr.row_offsets.size() * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_col_indices, csr.col_indices.size() * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_weights, csr.weights.size() * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_pheromones, h_pheromones.size() * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_rng_states, NUM_ANTS * sizeof(curandState)));
    CUDA_CHECK(cudaMalloc(&d_visited_nodes, NUM_ANTS * num_nodes * sizeof(bool)));
    CUDA_CHECK(cudaMalloc(&d_paths, NUM_ANTS * max_path_len * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_path_lengths, NUM_ANTS * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_path_costs, NUM_ANTS * sizeof(float)));

    CUDA_CHECK(cudaMemcpy(d_row_offsets, csr.row_offsets.data(), csr.row_offsets.size() * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_col_indices, csr.col_indices.data(), csr.col_indices.size() * sizeof(int), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_weights, csr.weights.data(), csr.weights.size() * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pheromones, h_pheromones.data(), h_pheromones.size() * sizeof(float), cudaMemcpyHostToDevice));

    const int num_blocks = (NUM_ANTS + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    init_rng_kernel<<<num_blocks, THREADS_PER_BLOCK>>>(d_rng_states, params.seed, NUM_ANTS);
    CUDA_CHECK(cudaGetLastError());

    Result result;
    result.bestPath.cost = Result::NO_PATH_COST;

    std::array<float, NUM_ANTS> h_path_costs;
    std::array<int, NUM_ANTS> h_path_lengths;

    for (int iter = 0; iter < params.iterations; ++iter) 
    {
        construct_paths_kernel<<<num_blocks, THREADS_PER_BLOCK>>>(
            d_row_offsets, d_col_indices, d_weights, num_nodes,
            params.alpha, params.beta, d_pheromones, d_rng_states,
            start, end, d_visited_nodes, d_paths, d_path_costs, d_path_lengths,
            max_path_len, NUM_ANTS);
        CUDA_CHECK(cudaGetLastError());

        CUDA_CHECK(cudaDeviceSynchronize());


        CUDA_CHECK(cudaMemcpy(h_path_costs.data(), d_path_costs, NUM_ANTS * sizeof(float), cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h_path_lengths.data(), d_path_lengths, NUM_ANTS * sizeof(int), cudaMemcpyDeviceToHost));
        
        int best_ant_iter = -1;
        float best_cost_iter = Result::NO_PATH_COST;
        for (int i = 0; i < NUM_ANTS; ++i) {
            if (h_path_lengths[i] > 0 && h_path_costs[i] < best_cost_iter) {
                best_cost_iter = h_path_costs[i];
                best_ant_iter = i;
            }
        }

        if (best_ant_iter != -1 && best_cost_iter < result.bestPath.cost) {
            result.bestPath.cost = best_cost_iter;
            result.bestPath.nodes.resize(h_path_lengths[best_ant_iter]);
            CUDA_CHECK(cudaMemcpy(result.bestPath.nodes.data(), d_paths + best_ant_iter * max_path_len, result.bestPath.nodes.size() * sizeof(int), cudaMemcpyDeviceToHost));
        }
        result.costPerIteration.push_back(result.bestPath.cost);

        const int matrix_size = num_nodes * num_nodes;
        const int evap_blocks = (matrix_size + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        evaporation_kernel<<<evap_blocks, THREADS_PER_BLOCK>>>(d_pheromones, matrix_size, params.evaporation, params.minimumPheromone);
        CUDA_CHECK(cudaGetLastError());

        deposit_kernel<<<num_blocks, THREADS_PER_BLOCK>>>(
            d_pheromones, d_paths, d_path_costs, d_path_lengths,
            result.bestPath.nodes.data(), result.bestPath.cost, static_cast<int>(result.bestPath.nodes.size()),
            params.depositBestOnly, max_path_len, num_nodes, NUM_ANTS);
        CUDA_CHECK(cudaGetLastError());
    }

    CUDA_CHECK(cudaFree(d_row_offsets));
    CUDA_CHECK(cudaFree(d_col_indices));
    CUDA_CHECK(cudaFree(d_weights));
    CUDA_CHECK(cudaFree(d_pheromones));
    CUDA_CHECK(cudaFree(d_rng_states));
    CUDA_CHECK(cudaFree(d_visited_nodes));
    CUDA_CHECK(cudaFree(d_paths));
    CUDA_CHECK(cudaFree(d_path_lengths));
    CUDA_CHECK(cudaFree(d_path_costs));

    return result;
}
} // namespace ACO
