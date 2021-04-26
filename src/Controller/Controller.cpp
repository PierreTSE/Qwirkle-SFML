#include "Controller/Controller.hpp"
#include "Engine/Utilities.hpp"
#include <array>
#include <unordered_set>

Controller::Controller(bool fillReserve) {
    if (fillReserve) {
        reserve.reserve(6 * 6 * 3);
        for (size_t i = 0; i < 6; ++i) {
            for (size_t j = 0; j < 6; ++j) {
                for (size_t k = 0; k < 3; ++k) {
                    reserve.emplace_back(i, (TileColor) j);
                }
            }
        }
        random_shuffle();
    }
}

void Controller::random_shuffle() {
    std::shuffle(reserve.begin(), reserve.end(), RandomEngine::instance());
}

std::vector<Tile> Controller::retrieveTilesFromReserve(size_t n) {
    std::vector<Tile> tiles;
    if (reserve.size() > n) {
        tiles.insert(tiles.end(), std::make_move_iterator(reserve.end() - n), std::make_move_iterator(reserve.end()));
        reserve.erase(reserve.end() - n, reserve.end());
    } else if (!reserve.empty()) {
        tiles.insert(tiles.end(), std::make_move_iterator(reserve.begin()), std::make_move_iterator(reserve.end()));
        reserve.clear();
    }
    return tiles;
}

bool Controller::emptyTile(sf::Vector2i const& coords) const {
    return map.find(coords) == map.end();
}

std::vector<sf::Vector2i> Controller::legitMoves(TileData const& tile) const {
    // std::vector<sf::Vector2i> legitMoves;
    std::vector<sf::Vector2i> potentialMoves;
    const std::array<sf::Vector2i, 4> directions = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
    for (auto const&[pos, v] : map) {
        // regarde toutes les tuiles de même couleur ou forme, non identiques
        if (!((v.color == tile.color || v.shapeID == tile.shapeID) && v != tile)) continue;
        // vérifie les cases alentours vides
        for (auto const& dir : directions) {
            const auto potentialPos = pos + dir;
            if (emptyTile(potentialPos)) {
                potentialMoves.emplace_back(potentialPos);
            }
        }
    }
    potentialMoves.erase(
            std::remove_if(potentialMoves.begin(), potentialMoves.end(), [tile, this](auto const& pos) { return !isMoveLegit(tile, pos); }),
            potentialMoves.end());
    return potentialMoves;
}

std::vector<sf::Vector2i> Controller::legitMoves(Tile const& tile) const {
    return legitMoves(TileData(tile));
}

bool Controller::isMoveLegit(TileData const& tile, sf::Vector2i const& pos) const {
    if (map.empty()) return true;
    if (!emptyTile(pos)) return false;
    // on suppose que le coup est légal puis on suit de gauche à droite à gauche et de haut en bas
    enum State {
        START, SHAPE_LINE, COLOR_LINE
    };
    const std::array<sf::Vector2i, 2> directions = {{{1, 0}, {0, 1}}};
    std::vector<std::unordered_set<TileData, TileDataHash>> sets(2);
    for (size_t i = 0; i < directions.size(); ++i) {
        auto const& dir = directions[i];
        auto curr_pos = pos;
        while (!emptyTile(curr_pos - dir)) curr_pos -= dir;
        if (curr_pos == pos) curr_pos += dir;
        State state = START;
        while (!emptyTile(curr_pos)) {
            auto const& curr_tile = map.at(curr_pos);
            if (curr_tile == tile) return false;
            switch (state) {
                case START:
                    if (curr_tile.color == tile.color) state = COLOR_LINE;
                    else if (curr_tile.shapeID == tile.shapeID) state = SHAPE_LINE;
                    else return false;
                    break;
                case SHAPE_LINE:
                    if (curr_tile.shapeID != tile.shapeID) return false;
                    break;
                case COLOR_LINE:
                    if (curr_tile.color != tile.color) return false;
                    break;
            }
            auto emplaced = sets[i].emplace(curr_tile);
            if (!emplaced.second) return false; // cette tuile existe déjà dans la ligne / colonne

            curr_pos += dir;
            if (curr_pos == pos) curr_pos += dir;
        }
    }
    return true;
}

bool Controller::isMoveLegit(Tile const& tile, sf::Vector2i const& pos) const {
    return isMoveLegit(TileData(tile), pos);
}

unsigned int Controller::score(std::vector<TileDataWithCoord> const& moves) const {
    // les moves doivent déjà être dans la carte logique


    // tout premier coup
    if (map.size() == 1 && moves.size() == 1 && map.find(moves.front().coord) != map.end()) return 1;

    /* 0 : left
     * 1 : right
     * 2 : up
     * 3 : down
     */
    const std::array<sf::Vector2i, 4> dirs = {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
    std::unordered_set<sf::Vector2i, Vector2Hash<int>> horizontal;
    std::unordered_set<sf::Vector2i, Vector2Hash<int>> vertical;
    unsigned int score = 0;
    for (auto const& tile : moves) {
        auto& move = tile.coord;
        // horizontal
        {
            auto min_x = move.x;
            while (!emptyTile(sf::Vector2i{min_x, move.y} + dirs[0])) min_x--;
            auto max_x = move.x;
            while (!emptyTile(sf::Vector2i{max_x, move.y} + dirs[1])) max_x++;
            if (min_x != max_x) {
                unsigned int qwirkle_count = 0;
                for (int x = min_x; x <= max_x; ++x) {
                    if (horizontal.find({x, move.y}) == horizontal.end()) {
                        score++;
                        qwirkle_count++;
                        horizontal.insert({x, move.y});
                    }
                }
                if (qwirkle_count == 6) score += 6;
            }
        }
        // vertical
        {
            auto min_y = move.y;
            while (!emptyTile(sf::Vector2i{move.x, min_y} + dirs[2])) min_y--;
            auto max_y = move.y;
            while (!emptyTile(sf::Vector2i{move.x, max_y} + dirs[3])) max_y++;
            if (min_y != max_y) {
                unsigned int qwirkle_count = 0;
                for (int y = min_y; y <= max_y; ++y) {
                    if (vertical.find({move.x, y}) == vertical.end()) {
                        score++;
                        qwirkle_count++;
                        vertical.insert({move.x, y});
                    }
                }
                if (qwirkle_count == 6) score += 6;
            }
        }
    }
    return score;
}

bool Controller::isConnectedToSomeIn(sf::Vector2i const& coords, std::vector<TileDataWithCoord> const& input, Controller const& controller) {
    if (input.empty()) return true;
    const bool alignsWithAll = std::find_if_not(input.begin(), input.end(), [&](auto const& e) { return e.coord.x == coords.x; }) == input.end() ||
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
