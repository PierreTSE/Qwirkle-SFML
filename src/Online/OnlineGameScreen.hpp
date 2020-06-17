#ifndef QWIRKLE_SFML_ONLINEGAMESCREEN_HPP
#define QWIRKLE_SFML_ONLINEGAMESCREEN_HPP

#include "Controller/Player.hpp"
#include "Engine/Screen.hpp"
#include "UI/Ui.hpp"
#include "View/Grid.hpp"
#include <SFML/Network.hpp>

class OnlineGameScreen : public Screen {
public:
    OnlineGameScreen(sf::RenderWindow& window, bool fillReserve);

    virtual void adapt_viewport(sf::RenderWindow& window) override;

protected:
    // player part
    Controller controller;

    Grid grid;

    Ui ui;
    sf::Text text;

    Tile* selectedTile = nullptr;
    void selectAtPos(size_t i, Player& player);

    bool recycleSelectionMode = false;
    sf::Sprite recycleFeedback;

    sf::Vector2i mouseLastPos;

    virtual void endTurnPlayer() = 0;

    // online
    uint16_t port;
    sf::SocketSelector selector;
};


#endif //QWIRKLE_SFML_ONLINEGAMESCREEN_HPP
