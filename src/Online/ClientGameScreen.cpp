#include "Online/ClientGameScreen.hpp"
#include "Engine/Localisator.hpp"
#include "Online/Commands.hpp"
#include "Online/DisconnectedException.hpp"
#include "View/TitleScreen.hpp"
#include "View/EndScreen.hpp"
#include <cassert>
#include <iostream>

ClientGameScreen::ClientGameScreen(sf::RenderWindow& window, std::unique_ptr<sf::TcpSocket> hostSocket, sf::Packet&& packet) :
        OnlineGameScreen(window, false),
        hostSocket{std::move(hostSocket)} {
    player = std::make_unique<Player>();

    // player data
    refillPlayerRack(packet, 6);
    player->replaceRack(window.getSize());

    // ui data
    uint8_t n; // players.size()
    packet >> n;
    for (size_t i = 0; i < n; ++i) {
        std::wstring name;
        uint8_t type;
        packet >> type;
        switch (type) {
            case 0: // host
                packet >> name;
                name = name + (name.empty() ? L"" : L" ") + L'[' + Localisator::get("Host") + L']';
                break;
            case 1: // client
                packet >> name;
                break;
            case 2: // ai
                packet >> type;
                name = Localisator::get("Computer") + L' ' + std::to_wstring(type);
                break;
        }
        scores.emplace_back(name, 0);
    }
    packet >> remainingTiles;

    selector.add(*ClientGameScreen::hostSocket);
}

void ClientGameScreen::adapt_viewport(sf::RenderWindow& window) {
    OnlineGameScreen::adapt_viewport(window);
    player->replaceRack(window.getSize());
}

std::unique_ptr<Screen> ClientGameScreen::execute() {
    try {
        while (window_.isOpen()) {
            sf::Event event;
            while (window_.pollEvent(event)) {
                auto result = manageEvent(event);
                if (result) return std::move(*result);

                switch (event.type) {

                    case sf::Event::MouseMoved: {
                        const auto mousePos = sf::Mouse::getPosition(window_);
                        if (selectedTile) {
                            selectedTile->setPosition(mousePos.x, mousePos.y);
                        } else {
                            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                                grid.move(static_cast<float>(event.mouseMove.x - mouseLastPos.x),
                                          static_cast<float>(event.mouseMove.y - mouseLastPos.y));
                                grid.updateTilesPositions();
                            }
                            if (isCurrentTurnToPlay) {
                                if (auto const&[rx, ry] = player->rack.getTileCoordinates(mousePos);
                                        (ui.isOnRecycleButton(mousePos) &&
                                         ((!recycleSelectionMode && player->moves.empty()) ||
                                          (recycleSelectionMode && std::find_if(player->rack.tiles.begin(), player->rack.tiles.end(),
                                                                                [](Tile const& t) { return t.shapeID == 6; })
                                                                   != player->rack.tiles.end()))) ||
                                        (ui.isOnNextTurnButton(mousePos) && !(player->moves.empty() || recycleSelectionMode)) ||
                                        (rx != -1 && ry != -1 &&
                                         ((!recycleSelectionMode && player->rack.tiles.size() > rx && player->rack.tiles.at(rx).disp) ||
                                          (recycleSelectionMode && std::find_if(player->rack.tiles.begin(), player->rack.tiles.end(),
                                                                                [&](Tile const& t) {
                                                                                    return t.shapeID == 6 && t.coord.x == rx;
                                                                                })
                                                                   == player->rack.tiles.end())))) {
                                    if (mouseCursor.setType(sf::Cursor::Type::Hand)) window_.setMouseCursor(mouseCursor);
                                } else {
                                    if (mouseCursor.setType(sf::Cursor::Type::Arrow)) window_.setMouseCursor(mouseCursor);
                                }
                            }
                        }
                        mouseLastPos = {event.mouseMove.x, event.mouseMove.y};
                    }
                        break;

                    case sf::Event::MouseButtonPressed: {
                        if (!isCurrentTurnToPlay) break;

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
                                            if (mouseCursor.setType(sf::Cursor::Type::Arrow)) window_.setMouseCursor(mouseCursor);
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
                        if (event.key.code != sf::Keyboard::C && !isCurrentTurnToPlay) break;

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


            // drawing
            window_.clear();
            window_.draw(bg_);

            grid.draw(window_);

            player->draw(window_);

            if (selectedTile) window_.draw(*selectedTile);

            ui.draw(window_);

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
            for (size_t i = 0; i < scores.size(); ++i) {
                text.setString(scores.at(i).first + L" : " + std::to_wstring(scores.at(i).second));
                text.setPosition(50, 70 + 50 * i);
                if (i == playing_idx) {
                    cursor.setPosition(text.getPosition().x - cursor.getGlobalBounds().width,
                                       text.getPosition().y + text.getGlobalBounds().height / 2);
                    window_.draw(cursor);
                    text.setStyle(sf::Text::Style::Bold);
                }
                window_.draw(text);
                if (i == playing_idx) text.setStyle(sf::Text::Style::Regular);
            }
            text.setString(Localisator::get("Remaining tiles") + L" : " + std::to_wstring(remainingTiles));
            text.setPosition(static_cast<float>(window_.getSize().x) - text.getGlobalBounds().width - 50, 20);
            window_.draw(text);

            window_.display();

            // online
            if (selector.wait(sf::milliseconds(10))) {
                if (selector.isReady(*hostSocket)) {
                    sf::Packet packet;
                    GameCommand command;
                    receive(packet);
                    if (!(packet >> command)) throw DisconnectedException();
                    uint8_t n;
                    Tile marker = Tile(6, Pink);
                    switch (command) {
                        case GameCommand::ClearMarkers:
                            // retire les coups des autres
                            grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                                            [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
                            break;
                        case GameCommand::EndGame:
                            packet >> n;
                            if (n) {
                                packet >> n;
                                packet >> scores.at(n).second;
                            }
                            return std::make_unique<EndScreen>(window_, std::move(scores), std::move(grid));
                        case GameCommand::Play:
                            isCurrentTurnToPlay = true;
                            if (playSoundOnTurnStart) soundOnTurnStart.play();
                            break;
                        case GameCommand::UpdatePlayingIdx:
                            packet >> playing_idx;
                            break;
                        case GameCommand::UpdateAfterMove:
                            // retire les coups des autres
                            grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                                            [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());
                            packet >> n; // moves.size()
                            for (size_t i = 0; i < n; ++i) {
                                TileDataWithCoord tileDataWithCoord;
                                packet >> tileDataWithCoord;
                                grid.tiles.emplace_back(tileDataWithCoord);
                                marker.coord = tileDataWithCoord.coord;
                                grid.tiles.emplace_back(marker);
                                controller.map.emplace(tileDataWithCoord.coord, static_cast<TileData>(tileDataWithCoord));
                            }
                            grid.updateTilesPositions();
                            packet >> n; // player_idx
                            packet >> scores.at(n).second;
                            packet >> remainingTiles;
                            break;
                        case GameCommand::UpdateScore:
                            packet >> n; // player_idx
                            packet >> scores.at(n).second;
                            break;
                        case GameCommand::UpdateScores:
                            for (auto& score : scores) packet >> score.second;
                            break;
                        case GameCommand::TurnPlayerToComputer:
                            packet >> n; // player_idx
                            scores.at(n).first = scores.at(n).first + L" [" + Localisator::get("Computer") + L']';
                            break;
                        default:
                            throw std::runtime_error("ClientGameScreen::execute() default switch case reached");
                    }
                }
            }

            sf::sleep(sf::milliseconds(10));
        }
    } catch (DisconnectedException&) {
        return disconnect();
    }

    return nullptr;
}

void ClientGameScreen::refillPlayerRack(sf::Packet& packet, uint8_t n) {
    std::vector<TileData> tiles;
    for (size_t i = 0; i < n; ++i) {
        TileData tile{};
        if (!(packet >> tile)) throw std::runtime_error("ClientGameScreen::refillPlayerRack : packet too small");
        player->rack.tiles.emplace_back(tile);
    }
    assert(player->rack.tiles.size() <= 6);
    std::sort(player->rack.tiles.begin(), player->rack.tiles.end());
    int i = 0;
    for (auto& tile : player->rack.tiles) tile.coord = {i++, 0};
    player->rack.updateTilesPositions();
}

void ClientGameScreen::toggleRecycleMode() {
    if (!recycleSelectionMode && !selectedTile && player->moves.empty()) {
        recycleSelectionMode = true;
        if (mouseCursor.setType(sf::Cursor::Type::Arrow)) window_.setMouseCursor(mouseCursor);
    } else if (recycleSelectionMode &&
               std::find_if(player->rack.tiles.begin(), player->rack.tiles.end(), [](Tile const& t) { return t.shapeID == 6; })
               != player->rack.tiles.end()) {
        recyclePlayer();
    }
}

void ClientGameScreen::recyclePlayer() {
    if (!recycleSelectionMode) return;
    recycleSelectionMode = false;
    isCurrentTurnToPlay = false;
    // recycle les tiles sélectionnées
    std::vector<size_t> selected;
    selected.reserve(6);
    // on récupère les indices des tiles sélectionnées
    for (auto const& tile : player->rack.tiles) if (tile.shapeID == 6) selected.push_back(tile.coord.x);
    // on les ajoute au paquet qui sera envoyé à la réserve, et on les marque à supprimer dans le rack
    sf::Packet packet;
    packet << GameCommand::Recycled;
    packet << static_cast<uint8_t>(selected.size());
    for (auto x : selected) {
        packet << static_cast<TileData>(player->rack.tiles.at(x));
        packet << static_cast<uint8_t>(x);
        player->rack.tiles.at(x).shapeID = 6;
    }
    // on les supprime dans le rack
    player->rack.tiles.erase(
            std::remove_if(player->rack.tiles.begin(), player->rack.tiles.end(), [](auto const& e) { return e.shapeID == 6; }),
            player->rack.tiles.end());
    // on refill le rack avec les tiles obtenues
    send(packet);
    packet.clear();
    receive(packet);
    GameCommand command;
    if (!(packet >> command)) throw DisconnectedException();
    if (command != GameCommand::RefillRack)
        throw std::runtime_error(
                "ClientGameScreen::recyclePlayer() : Bad command received instead of RefillRack : " +
                std::to_string(static_cast<uint8_t>(command)));
    refillPlayerRack(packet, selected.size());
    // passe la main
    playing_idx++;
}

void ClientGameScreen::endTurnPlayer() {
    if (player->moves.empty() || recycleSelectionMode) return;
    isCurrentTurnToPlay = false;
    // changement de curseur
    if (mouseCursor.setType(sf::Cursor::Type::Arrow)) window_.setMouseCursor(mouseCursor);
    // prépare le paquet
    sf::Packet packet;
    packet << GameCommand::Played;
    // enregistre les coups joués
    packet << static_cast<uint8_t>(player->moves.size());
    for (auto const& move : player->moves) {
        TileDataWithCoord tileDataWithCoord{move.second};
        packet << tileDataWithCoord;
    }
    // incrémente le score
    std::vector<TileDataWithCoord> moves;
    moves.reserve(player->moves.size());
    std::transform(player->moves.begin(), player->moves.end(), std::back_inserter(moves),
                   [](auto const& pair) { return TileDataWithCoord{pair.second}; });
    player->score += controller.score(moves);
    packet << player->score;
    send(packet);
    // màj le rack
    player->rack.tiles.erase(std::remove_if(player->rack.tiles.begin(), player->rack.tiles.end(),
                                            [](auto const& e) { return e.disp == false; }), player->rack.tiles.end());
    packet.clear();
    receive(packet);
    GameCommand command;
    if (!(packet >> command)) throw DisconnectedException();
    if (command != GameCommand::RefillRack)
        throw std::runtime_error(
                "ClientGameScreen::endTurnPlayer() : Bad command received instead of RefillRack : " +
                std::to_string(static_cast<uint8_t>(command)));
    uint8_t refillSize;
    packet >> refillSize;
    refillPlayerRack(packet, refillSize);
    // clear les coups joués
    player->moves.clear();
    // passe la main
    playing_idx++;
}

void ClientGameScreen::send(sf::Packet& packet) {
    auto code = hostSocket->send(packet);
    switch (code) {
        case sf::Socket::Done:
            return;
        case sf::Socket::NotReady:
            std::cerr << "ClientGameScreen::send NotReady" << std::endl;
            break;
        case sf::Socket::Partial:
            std::cerr << "ClientGameScreen::send Partial" << std::endl;
            break;
        case sf::Socket::Disconnected:
            std::cerr << "ClientGameScreen::send Disconnected" << std::endl;
            break;
        case sf::Socket::Error:
            std::cerr << "ClientGameScreen::send Error" << std::endl;
            break;
    }
}

void ClientGameScreen::receive(sf::Packet& packet) {
    auto code = hostSocket->receive(packet);
    switch (code) {
        case sf::Socket::Done:
            return;
        case sf::Socket::NotReady:
            std::cerr << "ClientGameScreen::receive NotReady" << std::endl;
            break;
        case sf::Socket::Partial:
            std::cerr << "ClientGameScreen::receive Partial" << std::endl;
            break;
        case sf::Socket::Disconnected:
            std::cerr << "ClientGameScreen::receive Disconnected" << std::endl;
            break;
        case sf::Socket::Error:
            std::cerr << "ClientGameScreen::receive Error" << std::endl;
            break;
    }
}

std::unique_ptr<Screen> ClientGameScreen::disconnect() {
    std::cerr << "You have been disconnected from the host" << std::endl;
    return std::make_unique<TitleScreen>(window_);
}
