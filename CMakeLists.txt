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

# Map library
add_library(map_lib
    src/map.cpp
)
target_include_directories(map_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Utils library
add_library(utils_lib INTERFACE
    ${PROJECT_SOURCE_DIR}/include/utils.hpp
)

target_link_libraries(map_lib PRIVATE graph_lib)
target_link_libraries(aco_lib PRIVATE graph_lib utils_lib)


# Unit tests
enable_testing()
add_subdirectory(tests)

#  Main executable
add_executable(aco_main
    src/main.cpp
)
target_link_libraries(aco_main PRIVATE graph_lib aco_lib map_lib)

include(CheckLanguage)

# CUDA support
check_language(CUDA)
if (CMAKE_CUDA_COMPILER)
    message(STATUS "CUDA compiler found: ${CMAKE_CUDA_COMPILER}")
    
    enable_language(CUDA)
    target_compile_definitions(aco_main PRIVATE USE_CUDA)
    
    
    add_subdirectory(src/cuda)
    set_target_properties(aco_main cuda_kernels PROPERTIES CUDA_ARCHITECTURES "86;89")  # https://developer.nvidia.com/cuda-gpus
    
    target_link_libraries(aco_main PRIVATE cuda_kernels)
else ()
    message(STATUS "CUDA compiler not found!")
endif()
