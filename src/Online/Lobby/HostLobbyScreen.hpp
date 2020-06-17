#ifndef QWIRKLE_SFML_HOSTLOBBYSCREEN_HPP
#define QWIRKLE_SFML_HOSTLOBBYSCREEN_HPP

#include "Engine/Screen.hpp"
#include "Online/Client.hpp"


class HostLobbyScreen : public Screen {
public:
    explicit HostLobbyScreen(sf::RenderWindow& window);
    static std::unique_ptr<HostLobbyScreen> factory(sf::RenderWindow& window);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:

    // interface
    sf::CircleShape cursor;
    uint8_t cursorPos = 0;

    sf::Text localIPText;
    sf::Text publicIPText;
    sf::Text namerhsText;
    sf::Text namelhsText;
    sf::Text playersText;
    sf::Text clearPlayersText;
    sf::Text clearComputersText;
    sf::Text addComputerText;
    sf::Text startText;
    sf::Text nameText;

    sf::RectangleShape bgNames;

    std::vector<std::wstring> computers;

    // online
    uint16_t port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<std::unique_ptr<Client>> clients;


    void resizeBgNames();
    void drawPlayerNames();
    void updateAndDrawCursor();

    void addComputer();
    void setPublicIPAddressText();
    void clearPlayers();
    void clearComputers();
    void broadcastLobbyState();
    void broadcastHostName();
    std::unique_ptr<Screen> startOnlineGame();
};


#endif //QWIRKLE_SFML_HOSTLOBBYSCREEN_HPP
