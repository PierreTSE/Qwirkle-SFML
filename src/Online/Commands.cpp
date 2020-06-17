#include "Online/Commands.hpp"

sf::Packet& operator<<(sf::Packet& packet, const LobbyCommand& command) {
    return packet << static_cast<uint8_t>(command);
}

sf::Packet& operator>>(sf::Packet& packet, LobbyCommand& command) {
    uint8_t i;
    packet >> i;
    command = static_cast<LobbyCommand>(i);
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const GameCommand& command) {
    return packet << static_cast<uint8_t>(command);
}

sf::Packet& operator>>(sf::Packet& packet, GameCommand& command) {
    uint8_t i;
    packet >> i;
    command = static_cast<GameCommand>(i);
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const TileData& tileData) {
    return packet << tileData.shapeID << static_cast<uint8_t>(tileData.color);
}

sf::Packet& operator>>(sf::Packet& packet, TileData& tileData) {
    packet >> tileData.shapeID;
    uint8_t i;
    packet >> i;
    tileData.color = static_cast<TileColor>(i);
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const TileDataWithCoord& tileDataWithCoord) {
    return packet
            << tileDataWithCoord.shapeID
            << static_cast<uint8_t>(tileDataWithCoord.color)
            << static_cast<int8_t>(tileDataWithCoord.coord.x)
            << static_cast<int8_t>(tileDataWithCoord.coord.y);
}

sf::Packet& operator>>(sf::Packet& packet, TileDataWithCoord& tileDataWithCoord) {
    uint8_t u;
    int8_t i;
    packet >> u;
    tileDataWithCoord.shapeID = u;
    packet >> u;
    tileDataWithCoord.color = static_cast<TileColor>(u);
    packet >> i;
    tileDataWithCoord.coord.x = i;
    packet >> i;
    tileDataWithCoord.coord.y = i;
    return packet;
}