#ifndef QWIRKLE_CONTROLLER_HPP
#define QWIRKLE_CONTROLLER_HPP

#include "Controller/Tile.hpp"
#include <unordered_map>
#include <vector>


template<typename T>
struct Vector2Hash {
    auto operator()(sf::Vector2<T> const& v) const noexcept {
        auto h1 = std::hash<T>{}(v.x);
        auto h2 = std::hash<T>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};

struct Controller {
    explicit Controller(bool fillReserve = true);

    // tile management
    std::vector<Tile> retrieveTilesFromReserve(size_t n);
    void random_shuffle();

    // game
    bool emptyTile(sf::Vector2i const& coords) const;
    bool isMoveLegit(TileData const& tile, sf::Vector2i const& coords) const;
    bool isMoveLegit(Tile const& tile, sf::Vector2i const& coords) const;
    std::vector<sf::Vector2i> legitMoves(TileData const& tile) const;
    std::vector<sf::Vector2i> legitMoves(Tile const& tile) const;
    unsigned int score(std::vector<TileDataWithCoord> const& moves) const;

    // members
    std::vector<Tile> reserve;
    std::unordered_map<sf::Vector2i, TileData, Vector2Hash<int>> map; // <2D coordinates, tile color & shape>
};


#endif //QWIRKLE_CONTROLLER_HPP
