#ifndef QWIRKLE_TILE_HPP
#define QWIRKLE_TILE_HPP

#include <SFML/Graphics.hpp>

enum TileColor {
    Red, Green, Blue, Purple, Orange, Yellow, White, Pink
};

sf::Color type2color(TileColor type);

struct Tile;

struct TileData {
    TileData() = default;
    TileData(unsigned char shapeID, TileColor color);
    explicit TileData(Tile const&);
    bool operator==(const TileData& rhs) const;
    bool operator!=(const TileData& rhs) const;
    unsigned char shapeID;
    TileColor color;
};

struct TileDataHash {
    auto operator()(TileData const& v) const noexcept {
        return std::hash<decltype(v.shapeID)>{}(v.shapeID) ^ (std::hash<decltype(v.color)>{}(v.color) << 1);
    }
};

struct TileDataWithCoord : TileData {
    TileDataWithCoord() = default;
    explicit TileDataWithCoord(Tile const& tile);
    sf::Vector2i coord;
    bool operator==(const Tile& rhs) const;
    TileDataWithCoord& operator=(const Tile& rhs);
    bool operator<(const TileDataWithCoord& rhs) const;
};

struct Tile : public sf::Sprite {
    Tile(unsigned char shapeID, TileColor color);

    Tile(TileDataWithCoord const& rhs);

    bool operator==(const Tile& rhs) const;
    bool operator!=(const Tile& rhs) const;
    bool operator==(const TileDataWithCoord& rhs) const;

    bool operator<(const Tile& rhs) const;

    unsigned char shapeID;
    TileColor color;

    sf::Vector2i coord;
    bool disp = true;
};


#endif //QWIRKLE_TILE_HPP
