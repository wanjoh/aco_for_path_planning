cmake_minimum_required(VERSION 3.12)
project(ACOPathPlanner LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Unit tests
enable_testing()
add_subdirectory(tests)

#  Main executable
add_executable(aco_main
    src/main.cpp
)
target_link_libraries(aco_main PRIVATE graph_lib aco_lib)

# CUDA support
include(CheckLanguage)
check_language(CUDA)

if (CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    
    set(CMAKE_CUDA_STANDARD 20)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    
    add_library(cuda_kernels STATIC
    src/cuda/aco_kernel.cu
    )
    set_target_properties(aco_main cuda_kernels PROPERTIES CUDA_ARCHITECTURES "86;89")
    set_target_properties(cuda_kernels PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    
    # target_include_directories(cuda_kernels PUBLIC PUBLIC graph_lib)
    
    target_link_libraries(aco_main PRIVATE cuda_kernels)
else ()
    message(STATUS "CUDA compiler not found!")
endif()
