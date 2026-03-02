set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
find_package(CUDAToolKit REQUIRED)

add_library(cuda_kernels
    aco_cuda.cu
)

target_link_libraries(cuda_kernels
    PUBLIC
    CUDA::cudart
    CUDA::curand
    graph_lib
    aco_lib
)

target_include_directories(cuda_kernels PUBLIC ${PROJECT_SOURCE_DIR}/include)

set_target_properties(cuda_kernels PROPERTIES LINKER_LANGUAGE CXX)