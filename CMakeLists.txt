cmake_minimum_required(VERSION 3.12)
project(ACOPathPlanner LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force static linking for SFML to simplify dependencies
set(SFML_USE_STATIC_LIBS TRUE)
set(BUILD_SHARED_LIBS OFF)

# SFML for visualization
include(FetchContent)
FetchContent_Declare(
  sfml
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(sfml)

#  Graph library
add_library(graph_lib STATIC
    src/graph.cpp
)
target_include_directories(graph_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

#  ACO library
add_library(aco_lib STATIC
    src/aco.cpp
)
target_include_directories(aco_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Map library
add_library(map_lib STATIC
    src/map.cpp
)
target_include_directories(map_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Utils library
add_library(utils_lib INTERFACE)
target_include_directories(utils_lib INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Visualizer library
add_library(visualizer_lib STATIC
    src/visualizer.cpp
)

target_link_libraries(map_lib PRIVATE graph_lib)
target_link_libraries(aco_lib PRIVATE graph_lib utils_lib)
target_include_directories(visualizer_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(visualizer_lib PUBLIC sfml-graphics sfml-window sfml-system map_lib)

# Unit tests
enable_testing()
# add_subdirectory(tests)

#  Main executable
add_executable(aco_main
    src/main.cpp
)
target_link_libraries(aco_main PRIVATE graph_lib aco_lib map_lib visualizer_lib sfml-graphics sfml-window sfml-system)

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
