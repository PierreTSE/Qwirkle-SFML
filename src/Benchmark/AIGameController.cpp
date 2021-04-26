#include "AIGameController.hpp"
#include <chrono>
#include <iostream>
#include <thread>

AIGameController::AIGameController(const std::vector<ai::Strategy>& strategies) : strategies{strategies} {
    wins = std::vector(strategies.size(), 0);
}

void AIGameController::simulateGame() {
    // initialisation

    controller = Controller();
    std::vector<Ai> players;
    size_t player_idx = 0;
    for (int i = 0; i < strategies.size(); ++i) {
        auto const& strategy = strategies.at(i);
        players.emplace_back(Ai(L"", controller, strategy));
    }

    // jeu
    for (;;) {
        // l'IA joue
        Ai& ai = players.at(player_idx);
        auto moves = ai.play(controller);
        if (!moves.empty()) {
            for (auto const& move : moves) {
                controller.map.emplace(move.coord, static_cast<TileData>(move)); // ajoute le coup à la carte logique
            }
        } else {
            ai.recycle(controller);
        }
        // l'IA termine son tour
        // efface les coups joués du rack et le remplit
        ai.rack.tiles.erase(std::remove_if(ai.rack.tiles.begin(), ai.rack.tiles.end(),
                                           [](auto const& e) { return e.disp == false; }), ai.rack.tiles.end());
        ai.refillRack(ai.moves.size(), controller);
        // reset les coups joués
        ai.moves.clear();
        // vérifie la fin de jeu (l'IA a utilisé toutes ses tuiles)
        if (controller.reserve.empty() && ai.rack.tiles.empty()) break;
        // passe la main
        player_idx++;
        player_idx %= players.size();
        // vérifie la fin de jeu (le joueur suivant ne peut plus jouer ou recycler)
        if (controller.reserve.empty() && !players.at(player_idx).canPlay(controller)) break;
    }

    auto winner_idx = std::distance(players.begin(),
                                    std::max_element(players.begin(), players.end(), [](auto const& a, auto const& b) { return a.score < b.score; }));
    wins.at(winner_idx)++;
}

void simulateNgames(int N, std::vector<ai::Strategy> const& strategies, int& count, std::vector<size_t>& wins) {
    auto aiGameController = AIGameController(strategies);
    for (int i = 0; i < N; ++i) {
        aiGameController.simulateGame();
        count++;
    }
    std::transform(aiGameController.wins.begin(), aiGameController.wins.end(), wins.begin(), wins.begin(), [](auto a, auto b) { return a + b; });
}

void AIGameController::parseArgs(int argc, char** argv) {
    int N = -1;
    std::vector<ai::Strategy> strategies;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i++]) == "--ai-benchmark") {
            N = std::stoi(argv[i++]);
            while (i < argc) {
                strategies.emplace_back(ai::fromString(std::string(argv[i++])));
            }
        }
    }
    if (N == -1 || strategies.empty()) throw std::invalid_argument("Invalid AI benchmark arguments.");

    std::vector<size_t> wins(strategies.size(), 0);

    int count = 0;
    int progress = 0;
    std::vector<std::thread> pool;
    size_t n_threads = std::thread::hardware_concurrency();
    size_t step = N / n_threads;
    pool.reserve(n_threads - 1);
    for (int i = 0; i < n_threads - 1; ++i) {
        pool.emplace_back(simulateNgames, step, strategies, std::ref(count), std::ref(wins));
    }
    pool.emplace_back(simulateNgames, N - (n_threads - 1) * step, strategies, std::ref(count), std::ref(wins));
    for (auto& t : pool) t.detach();

    auto t1 = std::chrono::high_resolution_clock::now();
    while (count < N) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto t2 = std::chrono::high_resolution_clock::now();
        if (count - progress > N / 10) {
            std::cout <<
                      count << "/" << N <<
                      " (" << count - progress << " games in " <<
                      std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1e9 << " s)" <<
                      std::endl;
            progress = count;
            t1 = std::chrono::high_resolution_clock::now();
        }
    }

    for (size_t i = 0; i < strategies.size(); ++i) {
        std::cout << ai::strategyName(strategies.at(i)) << " " << wins.at(i) << std::endl;
    }
}
