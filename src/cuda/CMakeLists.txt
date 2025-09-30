set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

file(GLOB CUDA_SOURCES *.cu)
add_library(cuda_kernels STATIC ${CUDA_SOURCES})

set_target_properties(cuda_kernels PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
    
# target_include_directories(cuda_kernels PUBLIC graph_lib)