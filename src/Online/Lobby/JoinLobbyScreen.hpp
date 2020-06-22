#ifndef QWIRKLE_SFML_JOINLOBBYSCREEN_HPP
#define QWIRKLE_SFML_JOINLOBBYSCREEN_HPP

#include "Engine/Cursor.hpp"
#include "Engine/Screen.hpp"
#include "Online/Lobby/WaitingLobbyScreen.hpp"
#include <SFML/Network.hpp>

class JoinLobbyScreen : public Screen {
public:
    explicit JoinLobbyScreen(sf::RenderWindow& window);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:

    // interface
    sf::CircleShape cursor;
    uint8_t cursorPos = 0;
    Cursor mouseCursor;

    sf::Text namelhsText;
    sf::Text namerhsText;
    sf::Text hostIPlhsText;
    sf::Text hostIPrhsText;
    sf::Text joinText;

    sf::RectangleShape bgNames;

    // online
    uint16_t port;
    std::unique_ptr<sf::TcpSocket> hostSocket;


    void updateAndDrawCursor();
    std::unique_ptr<WaitingLobbyScreen> joinHost();
};


#endif //QWIRKLE_SFML_JOINLOBBYSCREEN_HPP
