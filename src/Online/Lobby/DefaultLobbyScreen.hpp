#ifndef QWIRKLE_SFML_DEFAULTLOBBYSCREEN_HPP
#define QWIRKLE_SFML_DEFAULTLOBBYSCREEN_HPP

#include "Engine/Screen.hpp"

class DefaultLobbyScreen : public Screen {
public:
    explicit DefaultLobbyScreen(sf::RenderWindow& window);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    sf::CircleShape cursor;
    uint8_t cursorPos = 0;

    sf::Text createLobbyText;
    sf::Text joinLobbyText;

    void updateAndDrawCursor();
};


#endif //QWIRKLE_SFML_DEFAULTLOBBYSCREEN_HPP
