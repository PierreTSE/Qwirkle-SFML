#include "Controller/Ai.hpp"
#include "Engine/Settings.hpp"
#include "Engine/Utilities.hpp"
#include <iostream>
#include <chrono>
#include <utility>

void Ai::initializeStrategyFromConfig() {
    if (auto setting = Settings::get("aiStrategy")) {
        auto str = std::get<std::string>(setting.value());
        if (str == "RANDOM") strategy = ai::RANDOM;
        else if (str == "GREEDY") strategy = ai::GREEDY;
        else if (str == "MINMAX") strategy = ai::MINMAX;
        else throw std::invalid_argument("Unknown AI strategy : " + str);
    }
}

Ai::Ai(std::wstring name) {
    Player::name = std::move(name);
    initializeStrategyFromConfig();
}

Ai::Ai(std::wstring name, Controller& controller, sf::Vector2u const& windowSize)
        : Player(std::move(name), controller, windowSize) {
    initializeStrategyFromConfig();
}

Ai::Ai(std::wstring name, Controller& controller, ai::Strategy strategy)
        : Player(std::move(name), controller, {0, 0}) {
    this->strategy = strategy;
}

std::vector<TileDataWithCoord> Ai::play(Controller const& controller) {
    // cette fonction utilise des TileDataWithCoord pour éviter de devoir copier des sf::Sprite en manipulant des Tile

    std::vector<TileDataWithCoord> tilesPool; // rack de TileDataWithCoord
    tilesPool.insert(tilesPool.end(), rack.tiles.begin(), rack.tiles.end());

    Controller controllerCopy(false); // copie de controller
    controllerCopy.map = controller.map;

    std::pair<uint32_t, std::vector<TileDataWithCoord>> returned;
    switch (strategy) {
        case ai::RANDOM: returned = evaluateRandom(tilesPool, controllerCopy);
            break;
        case ai::GREEDY: returned = evaluateGreedy(tilesPool, controllerCopy);
            break;
        case ai::MINMAX: returned = evaluateMinmax(tilesPool, controllerCopy);
            break;
    }
    uint32_t max_score = returned.first;
    moves = returned.second;

    // marque les éléments à supprimer du rack
    for (auto const& move : moves) {
        auto it = std::find(rack.tiles.begin(), rack.tiles.end(), move);
        if (it != rack.tiles.end()) it->disp = false;
    }
    score += max_score;
    // std::cout << "score ordi : " << max_score << std::endl;
    hasPlayed = true;
    return moves;
}

std::pair<uint32_t, std::vector<TileDataWithCoord>> Ai::evaluateRandom(std::vector<TileDataWithCoord>& tilesPool, Controller& controllerCopy) const {

    std::pair<uint32_t, std::vector<TileDataWithCoord>> returned = {0, {}};

    std::shuffle(tilesPool.begin(), tilesPool.end(), RandomEngine::instance());
    std::pair<uint32_t, std::vector<TileDataWithCoord>> potential_moves = {0, {}};
    potential_moves.second.reserve(6);
    do {
        auto input = tilesPool;
        potential_moves.first = 0;
        potential_moves.second.clear();
        bestMove(controllerCopy, input, potential_moves);
        if (potential_moves.first > 0) {
            returned = potential_moves;
        }
    } while (std::next_permutation(tilesPool.begin(), tilesPool.end()));

    return returned;
}

std::pair<uint32_t, std::vector<TileDataWithCoord>> Ai::evaluateGreedy(std::vector<TileDataWithCoord>& tilesPool, Controller& controllerCopy) const {

    std::pair<uint32_t, std::vector<TileDataWithCoord>> returned = {0, {}};

    std::sort(tilesPool.begin(), tilesPool.end());
    do {
        auto input = tilesPool;
        std::pair<uint32_t, std::vector<TileDataWithCoord>> potential_moves = {0, {}};
        potential_moves.second.reserve(6);
        bestMove(controllerCopy, input, potential_moves);
        if (potential_moves.first > returned.first) {
            returned = potential_moves;
        }
    } while (std::next_permutation(tilesPool.begin(), tilesPool.end()));

    return returned;
}

std::pair<uint32_t, std::vector<TileDataWithCoord>> Ai::evaluateMinmax(std::vector<TileDataWithCoord>& tilesPool, Controller& controllerCopy) const {
}

void Ai::bestMove(Controller& controller, std::vector<TileDataWithCoord>& input, std::pair<uint32_t, std::vector<TileDataWithCoord>>& output) const {
    if (input.empty()) return;
    auto curr_tile = input.back();
    input.pop_back();
    // construit les moves possibles
    auto legitMoves = controller.legitMoves(curr_tile);
    legitMoves.erase(std::remove_if(legitMoves.begin(), legitMoves.end(),
                                    [&](auto const& e) { return !Controller::isConnectedToSomeIn(e, output.second, controller); }), legitMoves.end());

    std::pair<uint32_t, sf::Vector2i> currBestMove = {0, {-1, -1}}; // best <score, coord>
    if (controller.map.empty()) {
        // cas où l'ordi joue le tout premier coup
        currBestMove = {1, {49, 49}}; // hack le milieu de la grid de jeu est hardcodé
    } else {
        // on cherche la position qui rapporte le plus de points pour la tuile courante
        for (auto const& potentialMove : legitMoves) {
            curr_tile.coord = potentialMove;
            controller.map.emplace(potentialMove, curr_tile);
            output.second.emplace_back(curr_tile);
            auto score = controller.score(output.second);
            if (score > currBestMove.first) {
                currBestMove = {score, potentialMove};
            }
            controller.map.erase(potentialMove);
            output.second.pop_back();
        }
    }
    // si on ne trouve pas de coup qui fait progresser le score, on arrête de chercher pour cette combinaison ;
    // une autre combinaison dans le bon ordre passera sur les possibilités coupées ici
    if (currBestMove.first == 0) return;
    // sinon on ajoute notre coup et on continue récursivement
    controller.map.emplace(currBestMove.second, curr_tile);
    output.first = currBestMove.first;
    curr_tile.coord = currBestMove.second;
    output.second.emplace_back(curr_tile);
    bestMove(controller, input, output); // appel recursif
    controller.map.erase(currBestMove.second); // on clean le controller pour l'utiliser dans d'autres combinaisons
}

void Ai::recycle(Controller& controller) {
    // recycle toutes les tuiles
    auto toRefill = rack.tiles.size();
    controller.reserve.insert(controller.reserve.end(), std::make_move_iterator(rack.tiles.begin()), std::make_move_iterator(rack.tiles.end()));
    controller.random_shuffle();
    rack.tiles.clear();
    refillRack(toRefill, controller);
    rack.updateTilesPositions();
}

ClientType Ai::type() const {
    return ClientType::Ai;
}

std::string ai::strategyName(ai::Strategy strategy) {
    switch (strategy) {
        case RANDOM: return "RANDOM";
        case GREEDY: return "GREEDY";
        case MINMAX: return "MINMAX";
        default: throw std::invalid_argument("");
    }
}

ai::Strategy ai::fromString(std::string const& name) {
    if (name == "RANDOM") return RANDOM;
    else if (name == "GREEDY") return GREEDY;
    else if (name == "MINMAX") return MINMAX;
    else throw std::invalid_argument("Cannot convert to strategy :" + name);
}
