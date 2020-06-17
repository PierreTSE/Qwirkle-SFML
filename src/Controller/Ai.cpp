#include "Controller/Ai.hpp"
#include <iostream>
#include <chrono>

Ai::Ai(std::wstring name) {
    Player::name = std::move(name);
}

Ai::Ai(std::wstring name, Controller& controller, sf::Vector2u const& windowSize) : Player(std::move(name), controller, windowSize) {}

std::vector<TileDataWithCoord> Ai::play(Controller const& controller) {
    // cette fonction utilise des TileDataWithCoord pour éviter de devoir copier des sf::Sprite en manipulant des Tile

    uint32_t max_score = 0; // score max possible

    std::vector<TileDataWithCoord> tilesPool; // rack de TileDataWithCoord
    tilesPool.insert(tilesPool.end(), rack.tiles.begin(), rack.tiles.end());

    auto controllerCopy = controller; // copie de controller
    controllerCopy.reserve.clear();
    controllerCopy.reserve.shrink_to_fit();

    std::sort(tilesPool.begin(), tilesPool.end());
    do {
        auto input = tilesPool;
        std::pair<uint32_t, std::vector<TileDataWithCoord>> potential_moves = {0, {}};
        potential_moves.second.reserve(6);
        bestMove(controllerCopy, input, potential_moves);
        if (potential_moves.first > max_score) {
            max_score = potential_moves.first;
            moves = potential_moves.second;
        }
    } while (std::next_permutation(tilesPool.begin(), tilesPool.end()));

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

void Ai::bestMove(Controller& controller, std::vector<TileDataWithCoord>& input, std::pair<uint32_t, std::vector<TileDataWithCoord>>& output) {
    if (input.empty()) return;
    auto curr_tile = input.back();
    input.pop_back();
    // construit les moves possibles
    auto legitMoves = controller.legitMoves(curr_tile);
    legitMoves.erase(std::remove_if(legitMoves.begin(), legitMoves.end(),
                                    [&](auto const& e) { return !isConnectedToSomeIn(e, output.second, controller); }), legitMoves.end());


    std::pair<uint32_t, sf::Vector2i> currBestMove = {0, {-1, -1}}; // best <score, coord>
    if (controller.map.empty()) {
        // cas où l'ordi joue le tout premier coup
        currBestMove = {1, {49, 49}};
        // hack le milieu de la grid de jeu est hardcodé
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

bool Ai::isConnectedToSomeIn(sf::Vector2i const& coords, std::vector<TileDataWithCoord> const& input, Controller const& controller) {
    if (input.empty()) return true;
    bool alignsWithAll = std::find_if_not(input.begin(), input.end(), [&](auto const& e) { return e.coord.x == coords.x; }) == input.end() ||
                         std::find_if_not(input.begin(), input.end(), [&](auto const& e) { return e.coord.y == coords.y; }) == input.end();
    if (!alignsWithAll) return false;

    // vérifie que l'on peut tracer une ligne sans trou jusqu'à l'un des coups
    for (auto const& move : input) {
        auto const& goal_pos = move.coord;
        if (goal_pos.y == coords.y) {
            auto curr_pos = coords;
            while (curr_pos != goal_pos) {
                curr_pos.x += curr_pos.x < goal_pos.x ? 1 : -1;
                // si trou
                if (controller.emptyTile(curr_pos)) break;
            }
            if (curr_pos == goal_pos) return true;
        } else if (goal_pos.x == coords.x) {
            auto curr_pos = coords;
            while (curr_pos != goal_pos) {
                curr_pos.y += curr_pos.y < goal_pos.y ? 1 : -1;
                // si trou
                if (controller.emptyTile(curr_pos)) break;
            }
            if (curr_pos == goal_pos) return true;
        }
    }
    return false;
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
