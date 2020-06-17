#ifndef QWIRKLE_SFML_ENDSCREEN_HPP
#define QWIRKLE_SFML_ENDSCREEN_HPP

#include "Controller/Player.hpp"
#include "Engine/Screen.hpp"

class EndScreen : public Screen {
public:
    EndScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> const& players, Grid grid);
    EndScreen(sf::RenderWindow& window, std::vector<std::pair<std::wstring, uint16_t>> scores, Grid grid);

    void adapt_viewport(sf::RenderWindow& window) override;
    std::unique_ptr<Screen> execute() override;

private:
    void construct();

    sf::Text text;
    sf::Text gameEnded;

    std::vector<std::pair<std::wstring, uint16_t>> scores;
    Grid grid;

    sf::Vector2i mouseLastPos;
};


#endif //QWIRKLE_SFML_ENDSCREEN_HPP
