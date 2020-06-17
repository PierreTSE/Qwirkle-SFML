#ifndef QWIRKLE_SFML_WAITINGLOBBYSCREEN_HPP
#define QWIRKLE_SFML_WAITINGLOBBYSCREEN_HPP

#include "Engine/Screen.hpp"
#include <SFML/Network.hpp>

class WaitingLobbyScreen : public Screen {
public:
    explicit WaitingLobbyScreen(sf::RenderWindow& window, std::unique_ptr<sf::TcpSocket> hostSocket);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    // interface
    sf::CircleShape cursor;
    uint8_t cursorPos = 0;

    sf::Text waitingText;
    sf::Text playersText;
    sf::Text nameText;
    std::vector<std::wstring> names;

    sf::RectangleShape bgNames;

    // online
    uint16_t port;
    sf::SocketSelector selector;
    std::unique_ptr<sf::TcpSocket> hostSocket;


    void resizeBGHost();
    void drawPlayerNames();
};


#endif //QWIRKLE_SFML_WAITINGLOBBYSCREEN_HPP
