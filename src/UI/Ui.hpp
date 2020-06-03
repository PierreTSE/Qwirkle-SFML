#ifndef QWIRKLE_UI_HPP
#define QWIRKLE_UI_HPP

#include <SFML/Graphics.hpp>

class Ui : public sf::RectangleShape {
public:
    Ui();

    void draw(sf::RenderWindow& window) const;

    void replace(sf::Vector2u const& windowSize);

    bool isOnRecycleButton(sf::Vector2i const& pos);
    bool isOnNextTurnButton(sf::Vector2i const& pos);

private:
    sf::Sprite nextTurnButton;
    sf::Sprite recycleButton;
};


#endif //QWIRKLE_UI_HPP
