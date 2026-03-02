#pragma once
#include "map.hpp"
#include "aco.hpp"

namespace sf
{
class RenderWindow;
}

class Visualizer
{
public:
    explicit Visualizer(const Map& map, const ACO::Result& result);

    void run();

private:
    void drawGrid(sf::RenderWindow* window);
    void drawPath(sf::RenderWindow* window);

    const Map& m_map;
    const ACO::Result& m_result;
};
