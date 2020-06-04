#include "Player.hpp"
#include <iostream>

Player::Player(std::wstring name, Controller& controller, sf::Vector2u const& windowSize) :
        rack(40, 40, 1, 6, 5, sf::Color::White),
        name{std::move(name)} {
    rack.centerOrigin();
    refillRack(6, controller);
    replaceRack(windowSize);
    rack.scale(0.999, 0.999); // hack pour forcer l'activation de l'AA
}

void Player::draw(sf::RenderWindow& window) const {
    rack.draw(window);
}

void Player::replaceRack(sf::Vector2u const& windowSize) {
    rack.setPosition(static_cast<float>(windowSize.x) / 2, static_cast<float>(windowSize.y) - 75);
    rack.updateTilesPositions();
}

void Player::refillRack(size_t n, Controller& controller) {
    auto tiles = controller.retrieveTilesFromReserve(n);
    rack.tiles.insert(rack.tiles.end(), std::make_move_iterator(tiles.begin()), std::make_move_iterator(tiles.end()));
    std::sort(rack.tiles.begin(), rack.tiles.end());
    int i = 0;
    for (auto& tile : rack.tiles) tile.coord = {i++, 0};
}

void Player::recycle(Controller& controller) {
    // recycle les tiles sélectionnées
    std::vector<size_t> selected;
    selected.reserve(6);
    // on récupère les indices des tiles sélectionnées
    for (auto const& tile : rack.tiles) if (tile.shapeID == 6) selected.push_back(tile.coord.x);
    // on les ajoute à la réserve, et on les marque à supprimer dans le rack
    for (auto x : selected) {
        controller.reserve.emplace_back(rack.tiles.at(x));
        rack.tiles.at(x).shapeID = 6;
    }
    controller.random_shuffle();
    // on les supprime dans le rack
    rack.tiles.erase(
            std::remove_if(rack.tiles.begin(), rack.tiles.end(), [](auto const& e) { return e.shapeID == 6; }),
            rack.tiles.end());
    // on refill le rack du nombre retiré
    refillRack(selected.size(), controller);
    rack.updateTilesPositions();
}

void Player::addRecycleSelectionMarker(size_t pos) {
    if (std::find_if(rack.tiles.begin(), rack.tiles.end(),
                     [pos](const auto& e) {
                         return e.shapeID == 6 && e.coord.x == pos;
                     }) == rack.tiles.end()) {
        // std::cout << "bin selection at " << pos << std::endl;
        rack.tiles.emplace_back(6, White);
        rack.tiles.back().coord = {static_cast<int>(pos), 0};
        rack.updateTilesPositions();
    }
}

void Player::removeRecycleSelectionMarker() {
    rack.tiles.erase(std::remove_if(rack.tiles.begin(), rack.tiles.end(), [](auto const& e) { return e.shapeID == 6; }), rack.tiles.end());
}

void Player::updateTilesPositions() {
    rack.updateTilesPositions();
}

bool Player::isConnectedToAMove(sf::Vector2i const& coords, Controller const& controller) const {
    if (moves.empty()) return true;
    bool alignsWithAll = std::find_if_not(moves.begin(), moves.end(), [&](auto const& e) { return e.second.coord.x == coords.x; }) == moves.end() ||
                         std::find_if_not(moves.begin(), moves.end(), [&](auto const& e) { return e.second.coord.y == coords.y; }) == moves.end();
    if (!alignsWithAll) return false;

    auto map = controller.map;
    for (auto const& move : moves) map.emplace(move.second.coord, move.second);
    // vérifie que l'on peut tracer une ligne sans trou jusqu'à l'un des coups
    for (auto const& move : moves) {
        auto const& goal_pos = move.second.coord;
        if (goal_pos.y == coords.y) {
            auto curr_pos = coords;
            while (curr_pos != goal_pos) {
                curr_pos.x += curr_pos.x < goal_pos.x ? 1 : -1;
                // si trou
                if (map.find(curr_pos) == map.end()) break;
            }
            if (curr_pos == goal_pos) return true;
        } else if (goal_pos.x == coords.x) {
            auto curr_pos = coords;
            while (curr_pos != goal_pos) {
                curr_pos.y += curr_pos.y < goal_pos.y ? 1 : -1;
                // si trou
                if (map.find(curr_pos) == map.end()) break;
            }
            if (curr_pos == goal_pos) return true;
        }
    }
    return false;
}

bool Player::isAdjacentToSomeMove(sf::Vector2i const& coords) const {
    const std::array<sf::Vector2i, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
    for (auto const& move : moves) {
        for (auto const& dir : directions) {
            if (coords + dir == move.second.coord) return true;
        }
    }
    return false;
}

bool Player::canPlay(Controller const& controller) const {
    for (auto const& tile : rack.tiles) {
        auto positions = controller.legitMoves(tile);
        positions.erase(std::remove_if(positions.begin(), positions.end(),
                                       [&](auto const& e) { return !isConnectedToAMove(e, controller); }),
                        positions.end());
        if (!positions.empty()) return true;
    }
    return false;
}
