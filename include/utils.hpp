#pragma once

#include <cmath>
#include <limits>
#include <type_traits>

namespace utils 
{

static constexpr float EPS = 1e-5f;

template<typename T>
[[nodiscard]] constexpr bool isSimilar(T a, T b) 
{
    static_assert(std::is_floating_point_v<T>, "isSimilar only works for floating-point types.");
    return std::abs(a - b) < EPS;
}

} // namespace utils
