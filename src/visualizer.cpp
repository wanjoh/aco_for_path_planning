#include "visualizer.hpp"
#include <SFML/Graphics.hpp>

namespace
{
const sf::Color LIGHT_GRAY = sf::Color(200, 200, 200);
}

Visualizer::Visualizer(const Map& map, const ACO::Result& result)
    : m_map(map)
    , m_result(result)
{
}

void Visualizer::run()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "ACO Path Planning");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(LIGHT_GRAY);
        drawGrid(&window);
        drawPath(&window);
        window.display();
    }
}

void Visualizer::drawGrid(sf::RenderWindow* window)
{
    float windowWidth = static_cast<float>(window->getSize().x);
    float windowHeight = static_cast<float>(window->getSize().y);
    
    int rows = m_map.getHeight();
    int cols = m_map.getWidth();

    float cellWidth = windowWidth / cols;
    float cellHeight = windowHeight / rows;

    sf::RectangleShape cell(sf::Vector2f(cellWidth - 1.0f, cellHeight - 1.0f)); // -1 for the gap, maybe too hacky

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            cell.setPosition(c * cellWidth, r * cellHeight);
            
            char type = m_map.getCell(r, c);
            if (type == Map::OBSTACLE)
                cell.setFillColor(sf::Color::Black);
            else if (type == Map::START)
                cell.setFillColor(sf::Color::Green);
            else if (type == Map::END)
                cell.setFillColor(sf::Color::Red);
            else
                cell.setFillColor(sf::Color::White);

            window->draw(cell);
        }
    }
}

void Visualizer::drawPath(sf::RenderWindow* window)
{
    if (m_result.bestPath.nodes.size() < 2)
        return;

    float windowWidth = static_cast<float>(window->getSize().x);
    float windowHeight = static_cast<float>(window->getSize().y);
    
    int rows = m_map.getHeight();
    int cols = m_map.getWidth();

    float cellWidth = windowWidth / cols;
    float cellHeight = windowHeight / rows;

    sf::VertexArray lines(sf::LinesStrip);

    for (const auto& node : m_result.bestPath.nodes)
    {
        int r = node / m_map.getWidth();
        int c = node % m_map.getWidth();

        float x = c * cellWidth + cellWidth / 2.0f;
        float y = r * cellHeight + cellHeight / 2.0f;
        lines.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Blue));
    }
    window->draw(lines);
}
