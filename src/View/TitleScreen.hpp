#ifndef QWIRKLE_SFML_TITLESCREEN_HPP
#define QWIRKLE_SFML_TITLESCREEN_HPP

#include "Engine/Screen.hpp"

class TitleScreen : public Screen {
public:
    explicit TitleScreen(sf::RenderWindow& window);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    sf::Sprite qwirkle;
    sf::Text sfml;
    sf::Text playerText;
    sf::Text aiText;
    sf::Text startText;
    sf::Text onlineGameText;
    sf::CircleShape leftCursor;
    sf::CircleShape rightCursor;

    unsigned int players = 0, ai = 0, cursorPos = 0;

    void setPlayerText();
    void setAiText();
    void updateAndDrawCursor();

    void addParticipant();
    void removeParticipant();
    std::unique_ptr<Screen> switchToGameScreen();
    std::unique_ptr<Screen> switchToLobbyScreen();
};


#endif //QWIRKLE_SFML_TITLESCREEN_HPP
