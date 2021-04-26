#ifndef QWIRKLE_SFML_AI_HPP
#define QWIRKLE_SFML_AI_HPP

#include "Controller/Player.hpp"

namespace ai {
    enum Strategy {
        RANDOM, GREEDY, MINMAX
    };

    std::string strategyName(Strategy strategy);
    Strategy fromString(std::string const& name);
}

struct Ai : public Player {
    explicit Ai(std::wstring name);
    Ai(std::wstring name, Controller& controller, sf::Vector2u const& windowSize);
    Ai(std::wstring name, Controller& controller, ai::Strategy strategy);

    bool hasPlayed = false;
    std::vector<TileDataWithCoord> moves;

    std::vector<TileDataWithCoord> play(Controller const& controller);

    void recycle(Controller& controller) override;

    ClientType type() const override;

private:
    ai::Strategy strategy;
    void initializeStrategyFromConfig();
    std::pair<uint32_t, std::vector<TileDataWithCoord>> evaluateRandom(std::vector<TileDataWithCoord>&, Controller&) const;
    std::pair<uint32_t, std::vector<TileDataWithCoord>> evaluateGreedy(std::vector<TileDataWithCoord>&, Controller&) const;
    std::pair<uint32_t, std::vector<TileDataWithCoord>> evaluateMinmax(std::vector<TileDataWithCoord>&, Controller&) const;

    // return <score, moves>
    void bestMove(Controller& controller, std::vector<TileDataWithCoord>& input, std::pair<uint32_t, std::vector<TileDataWithCoord>>& output) const;
};


#endif //QWIRKLE_SFML_AI_HPP
