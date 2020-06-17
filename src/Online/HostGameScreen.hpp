#ifndef QWIRKLE_SFML_HOSTGAMESCREEN_HPP
#define QWIRKLE_SFML_HOSTGAMESCREEN_HPP

#include "Controller/Controller.hpp"
#include "Engine/Screen.hpp"
#include "Online/Client.hpp"
#include "Online/OnlineGameScreen.hpp"

class HostGameScreen : public OnlineGameScreen {
public:
    explicit HostGameScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> clients);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    // controller part
    std::vector<std::unique_ptr<Player>> players;
    uint8_t player_idx = 0;
    size_t hostIndex;

    void toggleRecycleMode();

    bool checkReserveValidity() const;

    void endTurnPlayer() override; // fin du tour de l'hôte
    void endTurnAi();
    void nextTurn(); // vérifie la fin de jeu et passe la main

    bool isGameEnded = false;
    std::unique_ptr<Screen> endGame();

    // online
    void send(std::unique_ptr<Player>& client, sf::Packet& packet);
    void broadcast(sf::Packet const& packet);
    void receive(std::unique_ptr<Player>& client, sf::Packet& packet);
    void disconnectClient(std::unique_ptr<Player>& client);
    void broadcastScoreOfCurrentPlayer();
    void broadcastScores();
};


#endif //QWIRKLE_SFML_HOSTGAMESCREEN_HPP
