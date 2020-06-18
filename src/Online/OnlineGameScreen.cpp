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

    mouseLastPos = sf::Mouse::getPosition();

    ui.replace(window.getSize());
    cursor.setPointCount(3);
    cursor.setRadius(10);
    centerOrigin(cursor);
    cursor.setRotation(90);

    recycleFeedback.setTexture(RessourceLoader::getTexture("sprites/bin.png"));
    centerOrigin(recycleFeedback);
    recycleFeedback.setScale(0.75, 0.75);

    text.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    text.setFillColor(sf::Color::White);

    if (auto setting = Settings::get("playSoundOnTurnStart")) playSoundOnTurnStart = std::get<bool>(setting.value());
    if (playSoundOnTurnStart) {
        soundOnTurnStart.setBuffer(RessourceLoader::getSoundBuffer("audio/turn.wav"));
    }
}

void OnlineGameScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
    if (selectedTile) selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
    grid.updateTilesPositions();
    ui.replace(window.getSize());
}

void OnlineGameScreen::toggleMarkers() {
    for (auto& tile : grid.tiles) {
        if (tile.shapeID == 6) {
            tile.disp = !tile.disp;
        }
    }
}

void OnlineGameScreen::selectAtPos(size_t i, Player& player) {
    if (!recycleSelectionMode) {
        if (player.rack.tiles.size() > i && player.rack.tiles.at(i).disp) {
            if (selectedTile) {
                selectedTile = nullptr;
                player.updateTilesPositions();
            }
            selectedTile = &player.rack.tiles.at(i);
            selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
        }
    } else {
        if (player.rack.tiles.size() > i) player.addRecycleSelectionMarker(i);
    }
}