#ifndef QWIRKLE_SFML_CLIENTGAMESCREEN_HPP
#define QWIRKLE_SFML_CLIENTGAMESCREEN_HPP

#include "Controller/Player.hpp"
#include "Online/OnlineGameScreen.hpp"
#include <SFML/Network.hpp>

class ClientGameScreen : public OnlineGameScreen {
public:
    ClientGameScreen(sf::RenderWindow& window, std::unique_ptr<sf::TcpSocket> hostSocket, sf::Packet&& packet);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    // player part
    std::unique_ptr<Player> player;
    bool isCurrentTurnToPlay = false;

    std::vector<std::pair<std::wstring, uint16_t>> scores;
    uint8_t playing_idx = 0;
    uint8_t remainingTiles;

    void refillPlayerRack(sf::Packet& packet, uint8_t n);
    void toggleRecycleMode();
    void recyclePlayer();
    void endTurnPlayer() override;

    // online
    std::unique_ptr<sf::TcpSocket> hostSocket;
    void send(sf::Packet& packet);
    void receive(sf::Packet& packet);
    std::unique_ptr<Screen> disconnect();
};


#endif //QWIRKLE_SFML_CLIENTGAMESCREEN_HPP
