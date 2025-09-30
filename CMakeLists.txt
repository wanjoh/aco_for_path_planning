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

include(CheckLanguage)

# CUDA support
check_language(CUDA)
if (CMAKE_CUDA_COMPILER)
    message(STATUS "CUDA compiler found: ${CMAKE_CUDA_COMPILER}")
    
    enable_language(CUDA)
    target_compile_definitions(aco_main PRIVATE USE_CUDA)
    
    set(CUDA_ARCHITECTURES "86;89")  # sets compute capability, more info https://developer.nvidia.com/cuda-gpus
    
    add_subdirectory(src/cuda)
    target_link_libraries(aco_main PRIVATE cuda_kernels)
else ()
    message(STATUS "CUDA compiler not found!")
endif()
