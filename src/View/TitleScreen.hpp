#ifndef QWIRKLE_SFML_TITLESCREEN_HPP
#define QWIRKLE_SFML_TITLESCREEN_HPP

#include "Screen.hpp"

class TitleScreen : public Screen {
public:
    explicit TitleScreen(sf::RenderWindow& window);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    sf::RectangleShape bg;
    sf::Sprite qwirkle;
    sf::Text sfml;
    sf::Text playerText;
    sf::Text aiText;
    sf::Text startText;
    sf::CircleShape cursor;

    unsigned players = 0, ai = 0, cursorPos = 0;

    void setPlayerText();
    void setAiText();
    void updateAndDrawCursor();
};


#endif //QWIRKLE_SFML_TITLESCREEN_HPP
