#ifndef QWIRKLE_SFML_AI_HPP
#define QWIRKLE_SFML_AI_HPP

#include "Player.hpp"

struct Ai : public Player {
    Ai(std::wstring name, Controller& controller, sf::Vector2u const& windowSize);

    bool hasPlayed = false;
    std::vector<TileDataWithCoord> moves;

    std::vector<TileDataWithCoord> play(Controller const& controller);

    void recycle(Controller& controller) override;

private:
    // return <score, moves>
    void bestMove(Controller& controller, std::vector<TileDataWithCoord>& input, std::pair<uint32_t, std::vector<TileDataWithCoord>>& output);

    static bool isConnectedToSomeIn(sf::Vector2i const& coords, std::vector<TileDataWithCoord> const& input, Controller const& controller);
};


#endif //QWIRKLE_SFML_AI_HPP
