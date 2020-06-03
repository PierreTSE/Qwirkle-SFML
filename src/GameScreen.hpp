#ifndef QWIRKLE_GAMESCREEN_HPP
#define QWIRKLE_GAMESCREEN_HPP

#include "Controller/Controller.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Screen.hpp"
#include "UI/Ui.hpp"
#include <utility>


enum PlayerType {
    HUMAN, AI
};

class GameScreen : public Screen {
public:
    GameScreen(sf::RenderWindow& window, std::vector<PlayerType> const& playerTypes);
    std::unique_ptr<Screen> execute() override;

protected:
    void adapt_viewport(sf::RenderWindow& window) override;

private:
    Controller controller;

    Grid grid;
    void addHints();
    void removeHints();

    std::vector<std::unique_ptr<Player>> players;
    size_t player_idx = 0;

    bool endGame = false;
    void endTurnPlayer(bool forced = false);
    void endTurnAi();

    Tile* selectedTile = nullptr;
    void selectAtPos(size_t i);

    sf::Vector2i mouseLastPos;

    bool recycleSelectionMode = false;
    void toggleRecycleMode();
    sf::Sprite recycleFeedback;

    sf::Time waitAfterAiTurnTime = sf::milliseconds(100000);
    sf::Time waitedAfterAiTurnTime = sf::Time::Zero;

    const sf::Color bgColor = {53, 101, 77};
    sf::RectangleShape bg;
    Ui ui;
    sf::Text text;

    // settings
    bool forceHints = false;
    bool showAiRack = false;
};


#endif // QWIRKLE_GAMESCREEN_HPP
