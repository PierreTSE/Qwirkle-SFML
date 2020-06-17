#include "Online/HostGameScreen.hpp"
#include "Controller/Ai.hpp"
#include "Engine/Localisator.hpp"
#include "Online/Commands.hpp"
#include "View/EndScreen.hpp"
#include <cassert>
#include <iostream>

HostGameScreen::HostGameScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> clients) :
        OnlineGameScreen(window, true),
        players{std::move(clients)} {
    auto controllerSize = controller.reserve.size();
    for (size_t i = 0; i < players.size(); ++i) {
        auto& player = players.at(i);
        switch (player->type()) {
            case ClientType::Host:
                hostIndex = i;
                player->refillRack(6, controller);
                player->rack.scale(0.999, 0.999);
                player->replaceRack(window.getSize());
                break;
            case ClientType::Client: {
                sf::Packet packet;
                packet << LobbyCommand::StartGame;

                auto tiles = controller.retrieveTilesFromReserve(6);
                player->rack.tiles = tiles;
                // tiles
                for (auto const& tile : tiles) packet << static_cast<TileData>(tile);
                // player names
                packet << static_cast<uint8_t>(players.size());
                for (auto& p : players) {
                    if (p->type() == ClientType::Ai) {
                        auto wstr = p->name.substr(p->name.find_last_of(L' ') + 1);
                        packet << static_cast<uint8_t>(2) << static_cast<uint8_t>(std::stoi(wstr));
                    } else {
                        if (p->type() == ClientType::Host) {
                            packet << static_cast<uint8_t>(0) << p->name;
                        } else {
                            packet << static_cast<uint8_t>(1) << p->name;
                        }
                    }
                }
                // remaining tiles
                packet << static_cast<uint8_t>(controllerSize - 6 * players.size());
                // sending
                send(player, packet);

                // add client's socket to selector
                selector.add(static_cast<Client&>(*player).socket);
            }
                break;
            case ClientType::Ai:
                player->refillRack(6, controller);
                break;
        }
    }
    players.at(hostIndex)->name = players.at(hostIndex)->name +
                                  (players.at(hostIndex)->name.empty() ? L"" : L" ") + L'[' + Localisator::get("Host") + L']';

    if (players.front()->type() == ClientType::Client) {
        sf::Packet packet;
        packet << GameCommand::Play;
        send(players.front(), packet);
    }
    assert(checkReserveValidity());
}

void HostGameScreen::adapt_viewport(sf::RenderWindow& window) {
    OnlineGameScreen::adapt_viewport(window);
    players.at(hostIndex)->replaceRack(window.getSize());
}

std::unique_ptr<Screen> HostGameScreen::execute() {
    while (window_.isOpen()) {
        auto& player = players.at(player_idx);
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);

            switch (event.type) {

                case sf::Event::MouseMoved: {
                    if (selectedTile) {
                        selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
                    } else if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        grid.move(static_cast<float>(event.mouseMove.x - mouseLastPos.x),
                                  static_cast<float>(event.mouseMove.y - mouseLastPos.y));
                        grid.updateTilesPositions();
                    }
                    mouseLastPos = {event.mouseMove.x, event.mouseMove.y};
                }
                    break;

                case sf::Event::MouseButtonPressed: {
                    if (player->type() != ClientType::Host) break;

                    switch (event.mouseButton.button) {
                        case sf::Mouse::Left: {
                            if (selectedTile) {
                                // clic gauche avec selectedTile
                                auto rackTileCoordinates =
                                        player->rack.getTileCoordinates(sf::Mouse::getPosition(window_));
                                if (rackTileCoordinates.x != -1 && rackTileCoordinates.y != -1) break;

                                auto gridTileCoordinates = grid.getTileCoordinates(sf::Mouse::getPosition(window_));
                                if (gridTileCoordinates.x != -1 && gridTileCoordinates.y != -1
                                    && player->isConnectedToAMove(gridTileCoordinates, controller)
                                    && controller.isMoveLegit(*selectedTile, gridTileCoordinates)) {
                                    // move effectué
                                    auto tileCoordOnRack = selectedTile->coord.x;
                                    selectedTile->coord = gridTileCoordinates;
                                    grid.tiles.emplace_back(*selectedTile);
                                    player->moves.emplace_back(tileCoordOnRack, *selectedTile);
                                    controller.map[gridTileCoordinates] = TileData(*selectedTile);
                                    selectedTile = nullptr;
                                    grid.updateTilesPositions();
                                    player->rack.tiles.at(tileCoordOnRack).disp = false;
                                }
                            } else {
                                // clic gauche quand pas de selectedTile
                                if (ui.isOnRecycleButton(sf::Mouse::getPosition(window_))) {
                                    toggleRecycleMode();
                                } else if (ui.isOnNextTurnButton(sf::Mouse::getPosition(window_))) {
                                    endTurnPlayer();
                                } else {
                                    auto rackTileCoordinates =
                                            player->rack.getTileCoordinates(sf::Mouse::getPosition(window_));
                                    if (rackTileCoordinates.x != -1 && rackTileCoordinates.y != -1
                                        && rackTileCoordinates.x < player->rack.tiles.size()) {
                                        if (recycleSelectionMode) {
                                            player->addRecycleSelectionMarker(rackTileCoordinates.x);
                                        } else {
                                            if (!player->rack.tiles.at(rackTileCoordinates.x).disp)
                                                break;
                                            selectedTile = &player->rack.tiles.at(rackTileCoordinates.x);
                                            selectedTile->setPosition(sf::Mouse::getPosition(window_).x, sf::Mouse::getPosition(window_).y);
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
                                player->updateTilesPositions();
                            } else if (recycleSelectionMode) {
                                // annule le mode sélection pour recyclage
                                player->removeRecycleSelectionMarker();
                                recycleSelectionMode = false;
                            } else {
                                if (player->moves.empty()) break;
                                // annule le coup précédent
                                auto lastMove = player->moves.back();
                                player->moves.pop_back(); // retire de la liste de coups
                                // replace sur le rack
                                player->rack.tiles.at(lastMove.first).disp = true;
                                player->rack.tiles.at(lastMove.first).coord = {int(lastMove.first), 0};
                                player->rack.updateTilesPositions();
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
                    if (event.key.code != sf::Keyboard::C && player->type() != ClientType::Host) break;

                    switch (event.key.code) {
                        case sf::Keyboard::C: {
                            grid.centerIn(window_.getSize());
                        }
                            break;
                        case sf::Keyboard::E: {
                            endTurnPlayer();
                        }
                            break;
                        case sf::Keyboard::R: {
                            toggleRecycleMode();
                        }
                            break;
                        case sf::Keyboard::T: {
                            toggleMarkers();
                        }
                            break;
                        case sf::Keyboard::Num1: {
                            selectAtPos(0, *player);
                        }
                            break;
                        case sf::Keyboard::Num2: {
                            selectAtPos(1, *player);
                        }
                            break;
                        case sf::Keyboard::Num3: {
                            selectAtPos(2, *player);
                        }
                            break;
                        case sf::Keyboard::Num4: {
                            selectAtPos(3, *player);
                        }
                            break;
                        case sf::Keyboard::Num5: {
                            selectAtPos(4, *player);
                        }
                            break;
                        case sf::Keyboard::Num6: {
                            selectAtPos(5, *player);
                        }
                            break;
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
        while (players.at(player_idx)->type() == ClientType::Ai && !isGameEnded) {
            auto& ai = static_cast<Ai&>(*players.at(player_idx));
            // retire les coups des autres
            grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                            [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
            auto moves = ai.play(controller);
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
                ai.recycle(controller);
            }
            endTurnAi();
        }

        // endgame
        if (isGameEnded) { if (auto nextscreen = endGame()) return nextscreen; }
            // online
        else {
            if (selector.wait(sf::milliseconds(10))) {
                for (auto& player : players) {
                    auto& client = static_cast<Client&>(*player);
                    if (player->type() == ClientType::Client && selector.isReady(client.socket)) {
                        sf::Packet packet;
                        GameCommand command;
                        receive(player, packet);
                        if (!(packet >> command)) {
                            disconnectClient(player);
                            continue;
                        }
                        switch (command) {
                            case GameCommand::Played: {
                                // retire les coups des autres
                                grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                                                [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
                                // update host and prepare broadcast packet to update clients
                                uint8_t movesSize;
                                packet >> movesSize; // moves.size()
                                sf::Packet broadcastPacket;
                                broadcastPacket << GameCommand::UpdateAfterMove;
                                broadcastPacket << movesSize;
                                Tile marker = Tile(6, Pink);
                                for (size_t i = 0; i < movesSize; ++i) {
                                    TileDataWithCoord tileDataWithCoord;
                                    packet >> tileDataWithCoord;
                                    broadcastPacket << tileDataWithCoord;
                                    grid.tiles.emplace_back(tileDataWithCoord);
                                    marker.coord = tileDataWithCoord.coord;
                                    grid.tiles.emplace_back(marker);
                                    controller.map.emplace(tileDataWithCoord.coord, static_cast<TileData>(tileDataWithCoord));
                                    player->rack.tiles.erase(std::find_if(player->rack.tiles.begin(), player->rack.tiles.end(),
                                                                          [&tileDataWithCoord](Tile const& tile) {
                                                                              return tile == tileDataWithCoord;
                                                                          }));
                                }
                                grid.updateTilesPositions();
                                packet >> player->score;
                                // refill rack of current client
                                auto tiles = controller.retrieveTilesFromReserve(movesSize);
                                player->rack.tiles.insert(player->rack.tiles.end(), tiles.begin(), tiles.end());
                                assert(player->rack.tiles.size() <= 6);
                                // online
                                broadcastPacket << static_cast<uint8_t>(player_idx);
                                broadcastPacket << player->score;
                                broadcastPacket << static_cast<uint8_t>(controller.reserve.size());
                                sf::Packet clientPacket;
                                clientPacket << GameCommand::RefillRack;
                                clientPacket << static_cast<uint8_t>(tiles.size());
                                for (auto const& tile : tiles) clientPacket << static_cast<TileData>(tile);

                                send(player, clientPacket);
                                broadcast(broadcastPacket);
                                nextTurn();
                                assert(checkReserveValidity());
                            }
                                break;

                            case GameCommand::Recycled: {
                                uint8_t selectedSize;
                                uint8_t x;
                                packet >> selectedSize;
                                for (size_t i = 0; i < selectedSize; ++i) {
                                    TileData tileData{};
                                    packet >> tileData;
                                    controller.reserve.emplace_back(Tile(tileData));
                                    packet >> x;
                                    player->rack.tiles.at(x) = std::move(player->rack.tiles.back());
                                    player->rack.tiles.pop_back();
                                }
                                controller.random_shuffle();
                                // local recycle
                                auto tiles = controller.retrieveTilesFromReserve(selectedSize);
                                player->rack.tiles.insert(player->rack.tiles.end(), tiles.begin(), tiles.end());
                                assert(player->rack.tiles.size() <= 6);
                                // online
                                sf::Packet refillPacket;
                                refillPacket << GameCommand::RefillRack;
                                for (auto const& tile : tiles) refillPacket << static_cast<TileData>(tile);
                                send(player, refillPacket);
                                nextTurn();
                                assert(checkReserveValidity());
                                sf::Packet broadcastPacket;
                                broadcastPacket << GameCommand::ClearMarkers;
                                broadcast(broadcastPacket);
                            }
                                break;

                            default:
                                throw std::runtime_error("HostGameScreen::execute() default switch case reached");
                        }
                    }
                }
            }
        }


        // drawing
        window_.clear();
        window_.draw(bg_);

        grid.draw(window_);

        players.at(hostIndex)->draw(window_); // rack de l'hôte

        if (selectedTile) window_.draw(*selectedTile);

        ui.draw(window_); // boutons de l'hôte

        if (recycleSelectionMode) {
            recycleFeedback.setPosition(sf::Mouse::getPosition(window_).x + 20, sf::Mouse::getPosition(window_).y + 20);
            window_.draw(recycleFeedback);
        }

        // scores
        text.setString(Localisator::get("Scores"));
        // text.setString(L"Hôte");
        text.setPosition(50, 20);
        text.setStyle(sf::Text::Style::Underlined);
        window_.draw(text);
        text.setStyle(sf::Text::Style::Regular);
        for (size_t i = 0; i < players.size(); ++i) {
            text.setString(players.at(i)->name + L" : " + std::to_wstring(players.at(i)->score));
            text.setPosition(50, 70 + 50 * i);
            if (i == player_idx) {
                cursor.setPosition(text.getPosition().x - cursor.getGlobalBounds().width, text.getPosition().y + text.getGlobalBounds().height / 2);
                window_.draw(cursor);
                text.setStyle(sf::Text::Style::Bold);
            }
            window_.draw(text);
            window_.draw(text);
            if (i == player_idx) text.setStyle(sf::Text::Style::Regular);
        }
        text.setString(Localisator::get("Remaining tiles") + L" : " + std::to_wstring(controller.reserve.size()));
        text.setPosition(static_cast<float>(window_.getSize().x) - text.getGlobalBounds().width - 50, 20);
        window_.draw(text);

        window_.display();

        sf::sleep(sf::milliseconds(10));
    }

    return nullptr;
}

void HostGameScreen::toggleRecycleMode() {
    assert(player_idx == hostIndex);
    if (!recycleSelectionMode && !selectedTile && players.at(player_idx)->moves.empty()) {
        recycleSelectionMode = true;
    } else if (recycleSelectionMode) {
        players.at(player_idx)->recycle(controller);
        recycleSelectionMode = false;
        nextTurn();
    }
}

void HostGameScreen::endTurnPlayer() {
    assert(player_idx == hostIndex);
    auto& host = *players.at(player_idx);
    if (host.moves.empty()) return;
    // prépare le paquet
    sf::Packet packet;
    packet << GameCommand::UpdateAfterMove;
    // retire les coups des autres
    grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                    [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
    // enregistre les coups joués
    Tile marker = Tile(6, Pink);
    packet << static_cast<uint8_t>(host.moves.size());
    for (auto const& move : host.moves) {
        packet << static_cast<TileDataWithCoord>(move.second);
        // ajoute les marqueurs de tuiles justes jouées
        marker.coord = move.second.coord;
        grid.tiles.emplace_back(marker);
    }
    grid.updateTilesPositions();
    // incrémente le score
    std::vector<TileDataWithCoord> moves;
    moves.reserve(host.moves.size());
    std::transform(host.moves.begin(), host.moves.end(), std::back_inserter(moves),
                   [](auto const& pair) { return TileDataWithCoord{pair.second}; });
    host.score += controller.score(moves);
    // màj le rack
    host.rack.tiles.erase(std::remove_if(host.rack.tiles.begin(), host.rack.tiles.end(),
                                         [](auto const& e) { return e.disp == false; }), host.rack.tiles.end());
    host.refillRack(host.moves.size(), controller);
    host.rack.updateTilesPositions();
    // online
    packet << static_cast<uint8_t>(hostIndex);
    packet << host.score;
    packet << static_cast<uint8_t>(controller.reserve.size());
    broadcast(packet);
    // clear les coups joués
    host.moves.clear();
    // passe la main
    nextTurn();
}

void HostGameScreen::endTurnAi() {
    auto ai = dynamic_cast<Ai*>(players.at(player_idx).get());
    ai->hasPlayed = false; // reset le fait qu'il a joué
    // efface les coups joués du rack et le remplit
    ai->rack.tiles.erase(std::remove_if(ai->rack.tiles.begin(), ai->rack.tiles.end(),
                                        [](auto const& e) { return e.disp == false; }), ai->rack.tiles.end());
    ai->refillRack(ai->moves.size(), controller);

    // online
    sf::Packet packet;
    if (!ai->moves.empty()) {
        packet << GameCommand::UpdateAfterMove;
        packet << static_cast<uint8_t>(ai->moves.size());
        for (auto const& move : ai->moves) packet << move;
        packet << static_cast<uint8_t>(player_idx);
        packet << ai->score;
        packet << static_cast<uint8_t>(controller.reserve.size());
    } else {
        packet << GameCommand::ClearMarkers;
    }
    broadcast(packet);

    // reset les coups joués
    ai->moves.clear();
    // passe la main
    nextTurn();
}

void HostGameScreen::nextTurn() {
    if (controller.reserve.empty() && players.at(player_idx)->rack.tiles.empty()) {
        isGameEnded = true;
        return;
    }
    // passe la main
    player_idx++;
    player_idx %= players.size();
    // vérifie la fin de jeu
    if (controller.reserve.empty() && !players.at(player_idx)->canPlay(controller)) {
        isGameEnded = true;
    } else {
        if (players.at(player_idx)->type() == ClientType::Client) {
            sf::Packet packet;
            packet << GameCommand::Play;
            send(players.at(player_idx), packet);
        }
        sf::Packet packet;
        packet << GameCommand::UpdatePlayingIdx << player_idx;
        broadcast(packet);
        if (player_idx == hostIndex && playSoundOnTurnStart) soundOnTurnStart.play();
    }
}

std::unique_ptr<Screen> HostGameScreen::endGame() {
    sf::Packet packet;
    packet << GameCommand::EndGame;
    if (players.at(player_idx)->rack.tiles.empty()) {
        players.at(player_idx)->score += 6;
        packet << static_cast<uint8_t>(1);
        packet << player_idx;
        packet << players.at(player_idx)->score;
    } else {
        packet << static_cast<uint8_t>(0);
    }
    broadcast(packet);
    return std::make_unique<EndScreen>(window_, players, grid);
}

void HostGameScreen::send(std::unique_ptr<Player>& client, sf::Packet& packet) {
    auto code = static_cast<Client&>(*client).socket.send(packet);
    switch (code) {
        case sf::Socket::Done:
            return;
        case sf::Socket::NotReady:
            std::cerr << "HostGameScreen::send NotReady" << std::endl;
            break;
        case sf::Socket::Partial:
            std::cerr << "HostGameScreen::send Partial" << std::endl;
            break;
        case sf::Socket::Disconnected:
            disconnectClient(client);
            break;
        case sf::Socket::Error:
            std::cerr << "HostGameScreen::send Error" << std::endl;
            break;
    }
}

void HostGameScreen::broadcast(sf::Packet const& packet) {
    for (auto& player : players)
        if (player->type() == ClientType::Client) {
            sf::Packet packetCopy = packet;
            send(player, packetCopy);
        }
}

void HostGameScreen::receive(std::unique_ptr<Player>& client, sf::Packet& packet) {
    auto code = static_cast<Client*>(client.get())->socket.receive(packet);
    switch (code) {
        case sf::Socket::Done:
            return;
        case sf::Socket::NotReady:
            std::cerr << "HostGameScreen::receive NotReady" << std::endl;
            break;
        case sf::Socket::Partial:
            std::cerr << "HostGameScreen::receive Partial" << std::endl;
            break;
        case sf::Socket::Disconnected:
            disconnectClient(client);
            break;
        case sf::Socket::Error:
            std::cerr << "HostGameScreen::receive Error" << std::endl;
            break;
    }
}

void HostGameScreen::disconnectClient(std::unique_ptr<Player>& client) {
    if (client->type() != ClientType::Client) return;
    std::wcerr << client->name << " has been disconnected" << std::endl;
    auto rack = std::move(client->rack);
    auto score = client->score;
    client = std::make_unique<Ai>(client->name + L" [" + Localisator::get("Computer") + L']');
    client->rack = rack;
    client->score = score;

    uint8_t idx = std::distance(players.begin(), std::find(players.begin(), players.end(), client));
    sf::Packet packet;
    packet << GameCommand::TurnPlayerToComputer;
    packet << idx;
    broadcast(packet);
}

void HostGameScreen::broadcastScoreOfCurrentPlayer() {
    sf::Packet packet;
    packet << GameCommand::UpdateScore;
    packet << player_idx;
    packet << players.at(player_idx)->score;
    broadcast(packet);
}

void HostGameScreen::broadcastScores() {
    sf::Packet packet;
    packet << GameCommand::UpdateScores;
    for (auto const& player : players) {
        packet << player->score;
    }
    broadcast(packet);
}

bool HostGameScreen::checkReserveValidity() const {
    const Controller newController;
    auto controlReserve = newController.reserve;
    auto currReserve = controller.reserve;
    for (auto const& player : players) {
        currReserve.insert(currReserve.end(), player->rack.tiles.begin(), player->rack.tiles.end());
    }
    for (auto const&[coord, tile] : controller.map) {
        currReserve.emplace_back(Tile(tile));
    }
    std::sort(controlReserve.begin(), controlReserve.end());
    std::sort(currReserve.begin(), currReserve.end());
    return controlReserve == currReserve;
}
