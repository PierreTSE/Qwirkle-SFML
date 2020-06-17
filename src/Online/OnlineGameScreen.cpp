#include "Online/OnlineGameScreen.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Settings.hpp"
#include "Engine/Utilities.hpp"

OnlineGameScreen::OnlineGameScreen(sf::RenderWindow& window, bool fillReserve) :
        Screen(window),
        controller(fillReserve),
        grid(40, 40, 100, 100, 5) {

    if (auto setting = Settings::get("onlinePort")) port = std::get<int>(setting.value());

    grid.centerOrigin();
    grid.centerIn(window.getSize());
    grid.scale(0.999, 0.999); // hack pour forcer l'activation de l'AA

    mouseLastPos = sf::Mouse::getPosition();

    ui.replace(window.getSize());

    recycleFeedback.setTexture(RessourceLoader::getTexture("sprites/bin.png"));
    centerOrigin(recycleFeedback);
    recycleFeedback.setScale(0.75, 0.75);

    text.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    text.setFillColor(sf::Color::White);
}

void OnlineGameScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
    if (selectedTile) selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
    grid.updateTilesPositions();
    ui.replace(window.getSize());
}

void OnlineGameScreen::selectAtPos(size_t i, Player& player) {
    if (!recycleSelectionMode && player.rack.tiles.size() > i && player.rack.tiles.at(i).disp) {
        if (selectedTile) {
            selectedTile = nullptr;
            player.updateTilesPositions();
        }
        selectedTile = &player.rack.tiles.at(i);
        selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
    }
}