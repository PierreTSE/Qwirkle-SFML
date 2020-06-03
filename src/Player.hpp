#ifndef QWIRKLE_PLAYER_HPP
#define QWIRKLE_PLAYER_HPP

#include "Controller/Controller.hpp"
#include "Grid.hpp"

struct Player {
    Player(std::wstring name, Controller& controller, sf::Vector2u const& windowSize);

    void replaceRack(sf::Vector2u const& windowSize);
    void refillRack(size_t n, Controller& controller);
    virtual void recycle(Controller& controller);
    void addRecycleSelectionMarker(size_t pos);
    void removeRecycleSelectionMarker();
    void updateTilesPositions();
    bool isConnectedToAMove(sf::Vector2i const& coords, Controller const& controller) const;
    bool canPlay(Controller const& controller) const;

    void draw(sf::RenderWindow& window) const;

    Grid rack;
    std::vector<std::pair<size_t, Tile>> moves; // pair<x-coord sur le rack, pièce jouée>

    uint32_t score = 0;

    std::wstring name;

private:
    bool isAdjacentToSomeMove(sf::Vector2i const& coords) const;
};


#endif //QWIRKLE_PLAYER_HPP
