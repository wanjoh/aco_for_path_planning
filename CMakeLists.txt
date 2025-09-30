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

#  Graph library
add_library(graph_lib
    src/graph.cpp
)
target_include_directories(graph_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

#  ACO library
add_library(aco_lib
    src/aco.cpp
)
target_include_directories(aco_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(tests)

#  Main executable
add_executable(aco_main
    src/main.cpp
)
target_link_libraries(aco_main PRIVATE cuda_kernels graph_lib aco_lib)
