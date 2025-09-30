#include <iostream>

int main() 
{
#ifdef USE_CUDA
    std::cout << "Cuda is supported!" << std::endl;
#else
    std::cout << "Cuda is not supported!" << std::endl;
#endif
    return 0;
}
