file(GLOB CUDA_SOURCES *.cu)

add_library(cuda_kernels STATIC ${CUDA_SOURCES})
set_target_properties(cuda_kernels PROPERTIES CUDA_SEPARABLE_COMPILATION ON)