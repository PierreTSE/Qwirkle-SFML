#include "RessourceLoader.hpp"
#include "Tile.hpp"
#include "Utilities.hpp"
#include <cassert>

Tile::Tile(unsigned short shapeID, TileColor color) : shapeID(shapeID), color(color) {
    assert(shapeID <= 6);
    setTexture(RessourceLoader::getTexture("sprites/" + std::to_string(shapeID) + ".png"));
    setColor(type2color(color));
    centerOrigin(*this);
}

Tile::Tile(const TileDataWithCoord& rhs) : Tile{rhs.shapeID, rhs.color} {
    coord = rhs.coord;
}

bool Tile::operator==(const Tile& rhs) const {
    return std::tie(shapeID, color) == std::tie(rhs.shapeID, rhs.color);
}

bool Tile::operator!=(const Tile& rhs) const {
    return !(rhs == *this);
}

bool Tile::operator==(const TileDataWithCoord& rhs) const {
    return std::tie(shapeID, color) == std::tie(rhs.shapeID, rhs.color);
}

bool Tile::operator<(const Tile& rhs) const {
    return std::tie(color, shapeID) < std::tie(rhs.color, rhs.shapeID);
}

sf::Color type2color(TileColor type) {
    switch (type) {
        case Red:return {255, 0, 0};
        case Green:return {54, 255, 9};
        case Blue:return {35, 153, 255};
        case Purple:return {208, 61, 255};
        case Orange:return {240, 124, 0};
        case Yellow:return {246, 255, 0};
        case White:return sf::Color::White;
        case Pink:return sf::Color{255, 0, 255};
    }
}

TileData::TileData(unsigned short shapeID, TileColor color) : shapeID(shapeID), color(color) {
    assert(shapeID < 6);
}

TileData::TileData(Tile const& other) {
    color = other.color;
    shapeID = other.shapeID;
}

bool TileData::operator==(const TileData& rhs) const {
    return std::tie(shapeID, color) == std::tie(rhs.shapeID, rhs.color);
}

bool TileData::operator!=(const TileData& rhs) const {
    return !(rhs == *this);
}

TileDataWithCoord::TileDataWithCoord(const Tile& tile) : TileData(tile), coord{tile.coord} {}

bool TileDataWithCoord::operator<(const TileDataWithCoord& rhs) const {
    return std::tie(color, shapeID) < std::tie(rhs.color, rhs.shapeID);
}

bool TileDataWithCoord::operator==(const Tile& rhs) const {
    return std::tie(color, shapeID) == std::tie(rhs.color, rhs.shapeID);
}

TileDataWithCoord& TileDataWithCoord::operator=(const Tile& rhs) {
    if (rhs == *this) return *this;
    shapeID = rhs.shapeID;
    color = rhs.color;
    coord = rhs.coord;
    return *this;
}
