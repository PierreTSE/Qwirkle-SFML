#ifndef QWIRKLE_SFML_COMMANDS_HPP
#define QWIRKLE_SFML_COMMANDS_HPP

#include "Controller/Tile.hpp"
#include <cstdint>
#include <SFML/Network/Packet.hpp>


enum class LobbyCommand : uint8_t {
    UpdateHostName, UpdateLobby, StartGame
};

enum class GameCommand : uint8_t {
    ClearMarkers, EndGame, Play, Played, RefillRack, Recycled, TurnPlayerToComputer, UpdateAfterMove, UpdatePlayingIdx, UpdateScore, UpdateScores
};

sf::Packet& operator<<(sf::Packet& packet, const LobbyCommand& command);

sf::Packet& operator>>(sf::Packet& packet, LobbyCommand& command);

sf::Packet& operator<<(sf::Packet& packet, const GameCommand& command);

sf::Packet& operator>>(sf::Packet& packet, GameCommand& command);

sf::Packet& operator<<(sf::Packet& packet, const TileData& tileData);

sf::Packet& operator>>(sf::Packet& packet, TileData& tileData);

sf::Packet& operator<<(sf::Packet& packet, const TileDataWithCoord& tileDataWithCoord);

sf::Packet& operator>>(sf::Packet& packet, TileDataWithCoord& tileDataWithCoord);

#endif //QWIRKLE_SFML_COMMANDS_HPP
