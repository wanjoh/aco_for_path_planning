cmake_minimum_required(VERSION 3.12)
project(ACOPathPlanner LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 20)

include_directories(include)

file(GLOB CUDA_SOURCES src/cuda/*.cu)
add_library(cuda_kernels STATIC ${CUDA_SOURCES})
set_target_properties(cuda_kernels PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

add_executable(aco_main
    src/main.cpp
    src/graph.cpp
    src/aco.cpp
)

target_link_libraries(aco_main PRIVATE cuda_kernels)
