#include "Engine/Screen.hpp"


Screen::Screen(sf::RenderWindow& window) : window_{window} {
    bg_.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    const sf::Color bgColor = {53, 101, 77};
    bg_.setFillColor(bgColor);
}

std::optional<std::unique_ptr<Screen>> Screen::manageEvent(const sf::Event& event) {
    if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
        return std::make_optional(std::unique_ptr<Screen>(nullptr));
    else if (event.type == sf::Event::Resized) {
        adapt_viewport(window_);
        return std::nullopt;
    }

    return std::nullopt;
}

void Screen::adapt_viewport(sf::RenderWindow& window) {
    window_.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
    bg_.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
}