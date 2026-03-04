#include "visualizer.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>


namespace
{
const sf::Color LIGHT_GRAY = sf::Color(200, 200, 200);
}

Visualizer::Visualizer(const Map& map, const ACO::Result& result)
    : m_map(map)
    , m_result(result)
    , m_currentIteration(static_cast<int>(result.pathsPerIteration.size()) - 1)
    , m_leftBtn(std::make_unique<sf::RectangleShape>())
    , m_rightBtn(std::make_unique<sf::RectangleShape>())
    , m_middleTextRect(std::make_unique<sf::RectangleShape>())
    , m_middleText(std::make_unique<sf::Text>())
    , m_rightArrow(std::make_unique<sf::ConvexShape>(3))
    , m_leftArrow(std::make_unique<sf::ConvexShape>(3))
    , m_font(std::make_unique<sf::Font>())
    , m_isInputActive(false)
{
    m_leftBtn->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_leftBtn->setPosition(BUTTON_PADDING, BUTTON_PADDING);
    m_leftBtn->setFillColor(sf::Color(100, 100, 100));
    m_leftBtn->setOutlineThickness(2);
    m_leftBtn->setOutlineColor(sf::Color::Black);
    m_rightBtn->setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_rightBtn->setPosition(WINDOW_WIDTH - BUTTON_WIDTH - BUTTON_PADDING, BUTTON_PADDING);
    m_rightBtn->setFillColor(sf::Color(100, 100, 100));
    m_rightBtn->setOutlineThickness(2);
    m_rightBtn->setOutlineColor(sf::Color::Black);

    sf::ConvexShape leftArrow(3);
    m_leftArrow->setPoint(0, sf::Vector2f(40, 25));
    m_leftArrow->setPoint(1, sf::Vector2f(80, 15));
    m_leftArrow->setPoint(2, sf::Vector2f(80, 35));
    m_leftArrow->setFillColor(sf::Color::White);
    m_rightArrow->setPoint(0, sf::Vector2f(WINDOW_WIDTH - 40, 25));
    m_rightArrow->setPoint(1, sf::Vector2f(WINDOW_WIDTH - 80, 15));
    m_rightArrow->setPoint(2, sf::Vector2f(WINDOW_WIDTH - 80, 35));
    m_rightArrow->setFillColor(sf::Color::White);

    // handle dumb sfml pathing in case of running from /build/debug
    const auto prefix = "../";
    int i = 0;
    std::string path = "fonts/arial.ttf";
    for (; i < 3; i++)
    {
        if (m_font->loadFromFile(path))
        {
            break;
        }
        path = prefix + path;
    }
    if (i == 3)    std::cerr << "Failed to load font" << std::endl;

    const std::pair<int, int> textRectPosition = {WINDOW_WIDTH / 2 - TEXT_WIDTH / 2, BUTTON_PADDING};
        
    m_middleText->setString("Iteracija " + std::to_string(m_currentIteration + 1));
    m_middleText->setFont(*m_font);
    m_middleText->setPosition(textRectPosition.first + TEXT_WIDTH / 2, textRectPosition.second + BUTTON_HEIGHT / 2);
    
    sf::FloatRect textRect = m_middleText->getLocalBounds();

    m_middleText->setOrigin(textRect.left + textRect.width / 2.0f,
               textRect.top + textRect.height / 2.0f - BUTTON_PADDING / 2);

    m_middleTextRect->setSize(sf::Vector2f(TEXT_WIDTH, BUTTON_HEIGHT));
    m_middleTextRect->setPosition(textRectPosition.first, textRectPosition.second);
    m_middleTextRect->setFillColor(sf::Color(100, 100, 100));
    m_middleTextRect->setOutlineThickness(2);
    m_middleTextRect->setOutlineColor(sf::Color::Black);
}

void Visualizer::run()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "ACO Path Planning");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else
                update(event, sf::Mouse::getPosition(window));
        }

        window.clear(LIGHT_GRAY);
        drawGrid(&window);
        drawPath(&window, m_currentIteration);
        drawUI(&window);
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
    float cellHeight = (windowHeight - HEADER_PADDING) / rows;

    sf::RectangleShape cell(sf::Vector2f(cellWidth - 1.0f, cellHeight - 1.0f)); // -1 for the gap, maybe too hacky

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            cell.setPosition(c * cellWidth, HEADER_PADDING + r*cellHeight);
            
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

void Visualizer::drawPath(sf::RenderWindow* window, int iteration)
{
    iteration = std::clamp(iteration, 0, static_cast<int>(m_result.pathsPerIteration.size()) - 1);

    const auto& path = m_result.pathsPerIteration[iteration];
    if (path.nodes.size() < 2)
        return;

    float windowWidth = static_cast<float>(window->getSize().x);
    float windowHeight = static_cast<float>(window->getSize().y);
    
    int rows = m_map.getHeight();
    int cols = m_map.getWidth();

    float cellWidth = windowWidth / cols;
    float cellHeight = (windowHeight - HEADER_PADDING) / rows;

    sf::VertexArray lines(sf::LinesStrip);

    for (const auto& node : path.nodes)
    {
        int r = node / m_map.getWidth();
        int c = node % m_map.getWidth();

        float x = c * cellWidth + cellWidth / 2.0f;
        float y = HEADER_PADDING + r * cellHeight + cellHeight / 2.0f;
        lines.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Blue));
    }
    window->draw(lines);
}

void Visualizer::drawUI(sf::RenderWindow *window)
{
    window->draw(*m_leftBtn);
   
    window->draw(*m_rightBtn);
    
    window->draw(*m_rightArrow);
    
    window->draw(*m_leftArrow);

    window->draw(*m_middleTextRect);
    
    window->draw(*m_middleText);
}

void Visualizer::update(const sf::Event& event, const sf::Vector2i& mousePos)
{
    if (isMouseClicked(event))
    {
        if (m_middleTextRect->getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            m_isInputActive = true;
            m_inputBuffer = std::to_string(m_currentIteration + 1);
            m_middleTextRect->setOutlineColor(sf::Color::Blue);
        }
        else
        {
            if (m_isInputActive)
            {
                m_isInputActive = false;
                m_middleTextRect->setOutlineColor(sf::Color::Black);
                try {
                    if (!m_inputBuffer.empty()) {
                        int val = std::stoi(m_inputBuffer);
                        m_currentIteration = std::clamp(val - 1, 0, static_cast<int>(m_result.pathsPerIteration.size()) - 1);
                    }
                } catch (...) {}
            }

            if (isButtonHovered(*m_leftBtn, mousePos))
            {
                m_currentIteration = std::max(0, m_currentIteration - 1);
            }
            else if (isButtonHovered(*m_rightBtn, mousePos))
            {
                m_currentIteration = std::min(static_cast<int>(m_result.pathsPerIteration.size()) - 1, m_currentIteration + 1);
            }
        }
    }
    else if (m_isInputActive && event.type == sf::Event::TextEntered)
    {
        if (event.text.unicode == '\b') // Backspace
        {
            if (!m_inputBuffer.empty())
                m_inputBuffer.pop_back();
        }
        else if (event.text.unicode == 13) // Enter
        {
            m_isInputActive = false;
            m_middleTextRect->setOutlineColor(sf::Color::Black);
            try {
                if (!m_inputBuffer.empty()) {
                    int val = std::stoi(m_inputBuffer);
                    m_currentIteration = std::clamp(val - 1, 0, static_cast<int>(m_result.pathsPerIteration.size()) - 1);
                }
            } catch (...) {}
        }
        else if (event.text.unicode >= '0' && event.text.unicode <= '9')
        {
            m_inputBuffer += static_cast<char>(event.text.unicode);
        }
    }

    if (m_isInputActive)
        m_middleText->setString("Iteracija " + m_inputBuffer);
    else
        m_middleText->setString("Iteracija " + std::to_string(m_currentIteration + 1));

    sf::FloatRect textRect = m_middleText->getLocalBounds();
    m_middleText->setOrigin(textRect.left + textRect.width / 2.0f,
               textRect.top + textRect.height / 2.0f - BUTTON_PADDING / 2);
}

inline bool Visualizer::isMouseClicked(const sf::Event &event)
{
    return event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left;
}

inline bool Visualizer::isButtonHovered(const sf::RectangleShape &button, const sf::Vector2i &mousePos)
{
    return button.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}
