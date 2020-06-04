#include "GameScreen.hpp"
#include "../Controller/Ai.hpp"
#include "EndScreen.hpp"
#include "GlobalClock.hpp"
#include "Localisator.hpp"
#include "RessourceLoader.hpp"
#include "Settings.hpp"
#include "Utilities.hpp"
#include <iostream>

GameScreen::GameScreen(sf::RenderWindow& window, std::vector<PlayerType> const& playerTypes) :
        Screen(window),
        // grid(40, 40, 3, 3, 5)
        grid(40, 40, 100, 100, 5) {
    grid.centerOrigin();
    grid.centerIn(window.getSize());
    grid.scale(0.999, 0.999); // hack pour forcer l'activation de l'AA

    size_t player = 0, ai = 0;
    for (auto const playerType : playerTypes) {
        players.emplace_back(playerType == HUMAN ? std::make_unique<Player>(Localisator::get("Player") + L" " + std::to_wstring(++player), controller,
                                                                            window_.getSize()) :
                             std::make_unique<Ai>(Localisator::get("Computer") + L" " + std::to_wstring(++ai), controller, window_.getSize()));
    }

    bg = sf::RectangleShape({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    bg.setFillColor(bgColor);

    mouseLastPos = sf::Mouse::getPosition();

    ui.replace(window.getSize());

    recycleFeedback.setTexture(RessourceLoader::getTexture("sprites/bin.png"));
    centerOrigin(recycleFeedback);
    recycleFeedback.setScale(0.75, 0.75);

    text.setFont(RessourceLoader::getFont("fonts/ubuntu-R.ttf"));
    text.setFillColor(sf::Color::White);

    // settings
    if (auto setting = Settings::get("forceHints")) forceHints = std::get<bool>(setting.value());
    if (auto setting = Settings::get("showAiRack")) showAiRack = std::get<bool>(setting.value());
    if (auto setting = Settings::get("waitAfterAiTurnTime")) waitAfterAiTurnTime = sf::milliseconds(std::get<int>(setting.value()));
}

void GameScreen::adapt_viewport(sf::RenderWindow& window) {
    window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
    bg.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    for (auto& player : players) player->replaceRack(window.getSize());
    if (selectedTile) selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
    grid.updateTilesPositions();
    ui.replace(window.getSize());
}

std::unique_ptr<Screen> GameScreen::execute() {
    while (window_.isOpen()) {
        GlobalClock::lap();

        auto& player = *players.at(player_idx);
        auto ai = dynamic_cast<Ai*>(&player);

        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);

            switch (event.type) {
                case sf::Event::MouseMoved: {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        grid.move(static_cast<float>(event.mouseMove.x - mouseLastPos.x),
                                  static_cast<float>(event.mouseMove.y - mouseLastPos.y));
                        grid.updateTilesPositions();
                    }
                    if (selectedTile) {
                        selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
                    }
                    mouseLastPos = {event.mouseMove.x, event.mouseMove.y};
                    // std::cout << event.mouseMove.x << " " << event.mouseMove.y << std::endl;
                }
                    break;
                case sf::Event::MouseButtonReleased: {
                    if (ai) break;
                    switch (event.mouseButton.button) {
                        case sf::Mouse::Left: {
                            if (selectedTile) {
                                // clic gauche avec selectedTile
                                auto rackTileCoordinates =
                                        player.rack.getTileCoordinates(sf::Mouse::getPosition(window_));
                                if (rackTileCoordinates.x != -1 && rackTileCoordinates.y != -1) break;

                                auto gridTileCoordinates = grid.getTileCoordinates(sf::Mouse::getPosition(window_));
                                if (gridTileCoordinates.x != -1 && gridTileCoordinates.y != -1
                                    && player.isConnectedToAMove(gridTileCoordinates, controller)
                                    && controller.isMoveLegit(*selectedTile, gridTileCoordinates)) {
                                    // move effectué
                                    auto tileCoordOnRack = selectedTile->coord.x;
                                    selectedTile->coord = gridTileCoordinates;
                                    grid.tiles.emplace_back(*selectedTile);
                                    player.moves.emplace_back(tileCoordOnRack, *selectedTile);
                                    controller.map[gridTileCoordinates] = TileData(*selectedTile);
                                    selectedTile = nullptr;
                                    removeHints();
                                    grid.updateTilesPositions();
                                    player.rack.tiles.at(tileCoordOnRack).disp = false;
                                }
                            } else {
                                // clic gauche quand pas de selectedTile
                                if (ui.isOnRecycleButton(sf::Mouse::getPosition(window_))) {
                                    toggleRecycleMode();
                                } else if (ui.isOnNextTurnButton(sf::Mouse::getPosition(window_))) {
                                    endTurnPlayer();
                                } else {
                                    auto rackTileCoordinates =
                                            player.rack.getTileCoordinates(sf::Mouse::getPosition(window_));
                                    if (rackTileCoordinates.x != -1 && rackTileCoordinates.y != -1
                                        && rackTileCoordinates.x < player.rack.tiles.size()) {
                                        if (recycleSelectionMode) {
                                            player.addRecycleSelectionMarker(rackTileCoordinates.x);
                                        } else {
                                            if (!player.rack.tiles.at(rackTileCoordinates.x).disp)
                                                break;
                                            selectedTile = &player.rack.tiles.at(rackTileCoordinates.x);
                                            selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
                                            if (forceHints) addHints();
                                        }
                                    }
                                }
                            }
                        }
                            break;
                        case sf::Mouse::Right: {
                            if (selectedTile) {
                                // range la tile courante
                                selectedTile = nullptr;
                                player.updateTilesPositions();
                                removeHints();
                            } else if (recycleSelectionMode) {
                                // annule le mode sélection pour recyclage
                                player.removeRecycleSelectionMarker();
                                recycleSelectionMode = false;
                            } else {
                                if (player.moves.empty()) break;
                                // annule le coup précédent
                                auto lastMove = player.moves.back();
                                player.moves.pop_back(); // retire de la liste de coups
                                // replace sur le rack
                                player.rack.tiles.at(lastMove.first).disp = true;
                                player.rack.tiles.at(lastMove.first).coord = {int(lastMove.first), 0};
                                player.rack.updateTilesPositions();
                                // retire la sprite de la grid d'affichage
                                grid.tiles.erase(
                                        std::find_if(grid.tiles.begin(), grid.tiles.end(), [lastMove](auto const& e) {
                                            return e.coord == lastMove.second.coord;
                                        }));
                                // retire de la map logique
                                controller.map.erase(lastMove.second.coord);
                            }
                        }
                            break;
                    }
                }
                    break;
                case sf::Event::KeyPressed: {
                    if (ai) {
                        switch (event.key.code) {
                            case sf::Keyboard::C:grid.centerIn(window_.getSize());
                                break;
                            default:waitedAfterAiTurnTime = waitAfterAiTurnTime;
                                break;
                        }
                    } else {
                        switch (event.key.code) {
                            case sf::Keyboard::C: {
                                grid.centerIn(window_.getSize());
                            }
                                break;
                            case sf::Keyboard::E: {
                                endTurnPlayer();
                            }
                                break;
                            case sf::Keyboard::H: {
                                addHints();
                            }
                                break;
                            case sf::Keyboard::R: {
                                toggleRecycleMode();
                            }
                                break;
                            case sf::Keyboard::Num1: {
                                selectAtPos(0);
                            }
                                break;
                            case sf::Keyboard::Num2: {
                                selectAtPos(1);
                            }
                                break;
                            case sf::Keyboard::Num3: {
                                selectAtPos(2);
                            }
                                break;
                            case sf::Keyboard::Num4: {
                                selectAtPos(3);
                            }
                                break;
                            case sf::Keyboard::Num5: {
                                selectAtPos(4);
                            }
                                break;
                            case sf::Keyboard::Num6: {
                                selectAtPos(5);
                            }
                                break;
                        }
                    }
                }
                    break;
                case sf::Event::MouseWheelMoved: {
                    auto scale = grid.getScale();
                    auto f = 1 + 0.05f * event.mouseWheel.delta;
                    grid.scale(f * scale.x, f * scale.y);
                    grid.centerOrigin();
                }
                    break;
            }
        }

        //gestion ai
        if (ai) {
            if (!ai->hasPlayed) {
                // retire les coups des IA
                grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                                [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
                auto moves = ai->play(controller);
                if (!moves.empty()) {
                    grid.tiles.insert(grid.tiles.end(), moves.begin(), moves.end());
                    std::transform(moves.begin(), moves.end(), std::back_inserter(grid.tiles), [](auto const& e) { return Tile{e}; });
                    Tile marker = Tile(6, Pink);
                    for (auto const& move : moves) {
                        marker.coord = move.coord; // marque les coups joués par l'ordinateur
                        grid.tiles.emplace_back(marker); // ajoute le coup à la grille visuelle
                        controller.map.emplace(move.coord, TileData{move}); // ajoute le coup à la carte logique
                    }
                    grid.updateTilesPositions();
                } else {
                    ai->recycle(controller);
                }
                waitedAfterAiTurnTime = sf::Time::Zero;
            }
            waitedAfterAiTurnTime += GlobalClock::lapTime();
            if (waitedAfterAiTurnTime >= waitAfterAiTurnTime) endTurnAi();
        }


        window_.clear();
        window_.draw(bg);

        grid.draw(window_);

        if (!ai || showAiRack) player.draw(window_); // rack du joueur

        if (selectedTile) window_.draw(*selectedTile);

        if (!ai) ui.draw(window_); // boutons pour joueur humain

        if (recycleSelectionMode) {
            recycleFeedback.setPosition(sf::Mouse::getPosition(window_).x + 20, sf::Mouse::getPosition(window_).y + 20);
            window_.draw(recycleFeedback);
        }

        // scores
        text.setString(Localisator::get("Scores"));
        text.setPosition(50, 20);
        text.setStyle(sf::Text::Style::Underlined);
        window_.draw(text);
        text.setStyle(sf::Text::Style::Regular);
        for (size_t i = 0; i < players.size(); ++i) {
            text.setString(players.at(i)->name + L" : " + std::to_wstring(players.at(i)->score));
            text.setPosition(50, 70 + 50 * i);
            if (i == player_idx) text.setStyle(sf::Text::Style::Bold);
            window_.draw(text);
            if (i == player_idx) text.setStyle(sf::Text::Style::Regular);
        }
        text.setString(Localisator::get("Remaining tiles") + L" : " + std::to_wstring(controller.reserve.size()));
        text.setPosition(static_cast<float>(window_.getSize().x) - text.getGlobalBounds().width - 50, 20);
        window_.draw(text);

        window_.display();

        if (endGame) return std::make_unique<EndScreen>(window_, std::move(players), grid);

        sf::sleep(sf::milliseconds(10));
    }

    return nullptr;
}

void GameScreen::addHints() {
    if (!selectedTile) return;
    auto hintPositions = controller.legitMoves(*selectedTile);
    hintPositions.erase(std::remove_if(hintPositions.begin(), hintPositions.end(),
                                       [&](auto const& e) { return !players.at(player_idx)->isConnectedToAMove(e, controller); }),
                        hintPositions.end());
    Tile hint(6, Yellow);
    for (auto const& pos : hintPositions) {
        hint.coord = pos;
        grid.tiles.emplace_back(hint);
    }
    grid.updateTilesPositions();
}


void GameScreen::removeHints() {
    grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                    [](auto const& e) { return e.shapeID == 6 && e.color == Yellow; }), grid.tiles.end());
}

void GameScreen::toggleRecycleMode() {
    if (!recycleSelectionMode && !selectedTile && players.at(player_idx)->moves.empty()) {
        recycleSelectionMode = true;
    } else if (recycleSelectionMode) {
        players.at(player_idx)->recycle(controller);
        recycleSelectionMode = false;
        endTurnPlayer(true);
    }
}

void GameScreen::endTurnPlayer(bool forced) {
    auto& player = *players.at(player_idx);
    if ((player.moves.empty() || recycleSelectionMode) && !forced) return;
    // enregistre les coups joués --> pas besoin, déjà fait au coup par coup
    // for (auto const& move : player.moves) controller.map[move.second.coord] = TileData(move.second);
    // incrémente le score
    std::vector<TileDataWithCoord> moves;
    moves.reserve(player.moves.size());
    std::transform(player.moves.begin(), player.moves.end(), std::back_inserter(moves),
                   [](auto const& pair) { return TileDataWithCoord{pair.second}; });
    player.score += controller.score(moves);
    // std::cout << "score joueur : " << controller.score(moves) << std::endl;
    // màj le rack
    player.rack.tiles.erase(std::remove_if(player.rack.tiles.begin(), player.rack.tiles.end(),
                                           [](auto const& e) { return e.disp == false; }), player.rack.tiles.end());
    player.refillRack(player.moves.size(), controller);
    player.rack.updateTilesPositions();
    // clear les coups joués
    player.moves.clear();
    // retire les coups des IA
    grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                    [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
    // passe la main
    player_idx++;
    player_idx %= players.size();
    // vérifie la fin de jeu
    if (controller.reserve.empty() && !players.at(player_idx)->canPlay(controller))
        endGame = true;
}

void GameScreen::endTurnAi() {
    auto ai = dynamic_cast<Ai*>(players.at(player_idx).get());
    ai->hasPlayed = false; // reset le fait qu'il a joué
    // efface les coups joués du rack et le remplit
    ai->rack.tiles.erase(std::remove_if(ai->rack.tiles.begin(), ai->rack.tiles.end(),
                                        [](auto const& e) { return e.disp == false; }), ai->rack.tiles.end());
    ai->refillRack(ai->moves.size(), controller);
    ai->rack.updateTilesPositions();
    // reset les coups joués
    ai->moves.clear();
    // passe la main
    player_idx++;
    player_idx %= players.size();
    // vérifie la fin de jeu
    if (controller.reserve.empty() && !players.at(player_idx)->canPlay(controller))
        endGame = true;
}

void GameScreen::selectAtPos(size_t i) {
    auto& player = *players.at(player_idx);
    if (!recycleSelectionMode && player.rack.tiles.size() > i && player.rack.tiles.at(i).disp) {
        if (selectedTile) {
            selectedTile = nullptr;
            player.updateTilesPositions();
            removeHints();
        }
        selectedTile = &player.rack.tiles.at(i);
        selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
        if (forceHints) addHints();
    }
}
