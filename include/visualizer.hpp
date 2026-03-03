#pragma once
#include "map.hpp"
#include "aco.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Font.hpp>


namespace sf
{
class RenderWindow;
class Event;
}

class Visualizer
{
public:
    explicit Visualizer(const Map& map, const ACO::Result& result);

    void run();

private:
    constexpr static int WINDOW_WIDTH = 800;
    constexpr static int HEADER_PADDING = 50;
    constexpr static int WINDOW_HEIGHT = 800 + HEADER_PADDING;
    void drawGrid(sf::RenderWindow* window);
    void drawPath(sf::RenderWindow* window, int iteration = std::numeric_limits<int>::max());
    void drawUI(sf::RenderWindow* window);
    void update(const sf::Event& event, const sf::Vector2i& mousePos);

    inline static bool isMouseClicked(const sf::Event& event);
    inline static bool isButtonHovered(const sf::RectangleShape& button, const sf::Vector2i& mousePos);

    const Map& m_map;
    const ACO::Result& m_result;

    // UI elements
    constexpr static int BUTTON_WIDTH = 100;
    constexpr static int BUTTON_HEIGHT = 30;
    constexpr static int BUTTON_PADDING = 10;
    constexpr static int TEXT_WIDTH = 200;
    std::unique_ptr<sf::RectangleShape> m_leftBtn;
    std::unique_ptr<sf::RectangleShape> m_rightBtn;
    std::unique_ptr<sf::ConvexShape> m_rightArrow;
    std::unique_ptr<sf::ConvexShape> m_leftArrow;
    std::unique_ptr<sf::Font> m_font;
    std::unique_ptr<sf::RectangleShape> m_middleTextRect;
    std::unique_ptr<sf::Text> m_middleText;
    int m_currentIteration;
};
