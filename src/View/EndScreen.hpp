#ifndef QWIRKLE_SFML_ENDSCREEN_HPP
#define QWIRKLE_SFML_ENDSCREEN_HPP

#include "../Controller/Player.hpp"
#include "Screen.hpp"

class EndScreen : public Screen {
public:
    explicit EndScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> players, Grid grid);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    sf::RectangleShape bg;
    sf::Text text;
    sf::Text gameEnded;

    std::vector<std::unique_ptr<Player>> players;
    Grid grid;

    sf::Vector2i mouseLastPos;
};


#endif //QWIRKLE_SFML_ENDSCREEN_HPP
