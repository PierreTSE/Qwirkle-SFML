#include "HostLobbyScreen.hpp"
#include "Controller/Ai.hpp"
#include "Engine/Localisator.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Settings.hpp"
#include "Engine/Utilities.hpp"
#include "Online/Commands.hpp"
#include "Online/Lobby/DefaultLobbyScreen.hpp"
#include "Online/HostGameScreen.hpp"
#include <iostream>

std::unique_ptr<HostLobbyScreen> HostLobbyScreen::factory(sf::RenderWindow& window) {
    auto screen = std::make_unique<HostLobbyScreen>(window);
    if (screen->listener.listen(screen->port) == sf::Socket::Done) {
        screen->selector.add(screen->listener);
        screen->setPublicIPAddressText();
        return screen;
    } else {
        std::cerr << "Could not listen to port " << screen->port << std::endl;
        return nullptr;
    }
}

HostLobbyScreen::HostLobbyScreen(sf::RenderWindow& window) : Screen(window) {
    if (auto setting = Settings::get("onlinePort")) port = std::get<int>(setting.value());

    cursor.setPointCount(3);
    cursor.setRadius(10);
    centerOrigin(cursor);
    cursor.setRotation(90);

    bgNames.setFillColor({0, 0, 0, 128});
    bgNames.setPosition(50, 170);

    localIPText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    localIPText.setCharacterSize(20);
    auto ip = sf::IpAddress::getLocalAddress();
    std::wstring wstr;
    if (ip != sf::IpAddress::None) {
        std::string str = ip.toString();
        wstr = std::wstring(str.begin(), str.end());
    } else wstr = Localisator::get("Cannot resolve IP");
    localIPText.setString(Localisator::get("Your local IP address") + L" : " + wstr);

    namelhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    namelhsText.setCharacterSize(20);
    namelhsText.setString(Localisator::get("Your name") + L" : ");
    namerhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    namerhsText.setCharacterSize(20);
    if (auto setting = Settings::get("name")) namerhsText.setString(std::get<std::string>(setting.value()));

    playersText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    playersText.setCharacterSize(20);
    playersText.setStyle(sf::Text::Style::Underlined);
    playersText.setString(Localisator::get("Players"));

    nameText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    nameText.setCharacterSize(20);

    clearPlayersText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    clearPlayersText.setCharacterSize(20);
    clearPlayersText.setString(Localisator::get("Remove all players"));
    clearComputersText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    clearComputersText.setCharacterSize(20);
    clearComputersText.setString(Localisator::get("Remove all computers"));
    addComputerText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    addComputerText.setCharacterSize(20);
    addComputerText.setString(Localisator::get("Add computer"));
    startText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    startText.setCharacterSize(20);
    startText.setString(L'[' + Localisator::get("Start") + L']');

    localIPText.setPosition(50, 50);
    publicIPText.setPosition(localIPText.getPosition().x, localIPText.getPosition().y + 30);
    namelhsText.setPosition(publicIPText.getPosition().x, publicIPText.getPosition().y + 30);
    namerhsText.setPosition(namelhsText.getPosition().x + namelhsText.getGlobalBounds().width, namelhsText.getPosition().y);
    playersText.setPosition(namelhsText.getPosition().x, namelhsText.getPosition().y + 30);

    HostLobbyScreen::adapt_viewport(window);
}

void HostLobbyScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);

    resizeBgNames();
}

std::unique_ptr<Screen> HostLobbyScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {

                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Left:
                            if (cursorPos > 0) cursorPos -= 1;
                            else cursorPos = 3;
                            break;
                        case sf::Keyboard::Right:
                            if (cursorPos < 3) cursorPos += 1;
                            else cursorPos = 0;
                            break;
                        case sf::Keyboard::BackSpace:
                            if (!namerhsText.getString().isEmpty()) {
                                std::string str = namerhsText.getString();
                                str.pop_back();
                                namerhsText.setString(std::move(str));
                            }
                            broadcastHostName();
                            break;
                        case sf::Keyboard::Enter:
                            switch (cursorPos) {
                                case 0:
                                    clearPlayers();
                                    break;
                                case 1:
                                    clearComputers();
                                    break;
                                case 2:
                                    addComputer();
                                    resizeBgNames();
                                    break;
                                case 3:
                                    if (auto nextscreen = startOnlineGame()) return nextscreen;
                                    break;
                            }
                            break;
                        case sf::Keyboard::Escape:
                            return std::make_unique<DefaultLobbyScreen>(window_);
                    }
                    break;

                case sf::Event::MouseButtonPressed : {
                    const auto posint = sf::Mouse::getPosition(window_);
                    const sf::Vector2f pos = {static_cast<float>(posint.x), static_cast<float>(posint.y)};
                    if (clearPlayersText.getGlobalBounds().contains(pos)) {
                        clearPlayers();
                    } else if (clearComputersText.getGlobalBounds().contains(pos)) {
                        clearComputers();
                    } else if (addComputerText.getGlobalBounds().contains(pos)) {
                        addComputer();
                        resizeBgNames();
                    } else if (startText.getGlobalBounds().contains(pos)) {
                        if (auto nextscreen = startOnlineGame()) return nextscreen;
                    }
                }
                    break;

                case sf::Event::TextEntered :
                    if (event.text.unicode < 32 && namerhsText.getString().getSize() < 50) break; // control chars
                    namerhsText.setString(namerhsText.getString().toWideString() + static_cast<wchar_t>(event.text.unicode));
                    broadcastHostName();
                    break;
            }
        }

        // drawing
        window_.clear();
        window_.draw(bg_);
        window_.draw(localIPText);
        window_.draw(publicIPText);
        window_.draw(namerhsText);
        window_.draw(namelhsText);
        drawPlayerNames();
        window_.draw(clearPlayersText);
        window_.draw(clearComputersText);
        window_.draw(addComputerText);
        window_.draw(startText);
        updateAndDrawCursor();
        window_.display();

        // online management
        if (selector.wait(sf::milliseconds(50))) {
            // nouvelle connexion
            if (selector.isReady(listener)) {
                auto client = std::make_unique<Client>();
                if (listener.accept(client->socket) == sf::Socket::Done) {
                    clients.emplace_back(std::move(client));
                    selector.add(clients.back()->socket);
                    resizeBgNames();
                }
            }
                // nouveau joueur
            else {
                auto it = clients.begin();
                while (it != clients.end()) {
                    auto const& client = *it;
                    if (selector.isReady(client->socket)) {
                        sf::Packet packet;
                        auto status = client->socket.receive(packet);
                        switch (status) {
                            case sf::Socket::Done:
                                // ajoute le nom du nouveau client
                                packet >> client->name;
                                ++it;
                                break;
                            case sf::Socket::Disconnected: {
                                std::string name(client->name.begin(), client->name.end());
                                std::cerr << name << " have been disconnected" << std::endl;
                                // supprime le client
                                client->socket.disconnect();
                                it = clients.erase(it);
                            }
                                break;
                            default:
                                ++it;
                        }
                    } else ++it;

                }
                broadcastLobbyState();
            }
        }
        sf::sleep(sf::milliseconds(50));
    }
    return nullptr;
}

void HostLobbyScreen::resizeBgNames() {
    bgNames.setSize({static_cast<float>(window_.getSize().x) - 100, static_cast<float>(clients.size() + computers.size() + 1) * 30.f});
    clearPlayersText.setPosition(80, bgNames.getPosition().y + bgNames.getSize().y);
    clearComputersText.setPosition(clearPlayersText.getPosition().x + clearPlayersText.getGlobalBounds().width + 50,
                                   bgNames.getPosition().y + bgNames.getSize().y);
    addComputerText.setPosition(clearComputersText.getPosition().x + clearComputersText.getGlobalBounds().width + 50,
                                bgNames.getPosition().y + bgNames.getSize().y);
    startText.setPosition(addComputerText.getPosition().x + addComputerText.getGlobalBounds().width + 50,
                          bgNames.getPosition().y + bgNames.getSize().y);
}

void HostLobbyScreen::drawPlayerNames() {
    window_.draw(playersText);
    window_.draw(bgNames);
    size_t pos = 0;
    nameText.setString(
            namerhsText.getString().toWideString() + (namerhsText.getString().isEmpty() ? L"" : L" ") + L'[' + Localisator::get("Host") + L']');
    nameText.setPosition(80, playersText.getPosition().y + 30.f * ++pos);
    window_.draw(nameText);
    for (auto const& client : clients) {
        nameText.setString(client->name);
        nameText.setPosition(80, playersText.getPosition().y + 30.f * ++pos);
        window_.draw(nameText);
    }
    for (auto const& name : computers) {
        nameText.setString(name);
        nameText.setPosition(80, playersText.getPosition().y + 30.f * ++pos);
        window_.draw(nameText);
    }
}

void HostLobbyScreen::updateAndDrawCursor() {
    switch (cursorPos) {
        case 0:
            cursor.setPosition(clearPlayersText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -clearPlayersText.getGlobalBounds().height / 2});
            break;
        case 1:
            cursor.setPosition(clearComputersText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -clearComputersText.getGlobalBounds().height / 2});
            break;
        case 2:
            cursor.setPosition(addComputerText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -addComputerText.getGlobalBounds().height / 2});
            break;
        case 3:
            cursor.setPosition(startText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -startText.getGlobalBounds().height / 2});
            break;
    }
    window_.draw(cursor);
}

void HostLobbyScreen::addComputer() {
    computers.emplace_back(Localisator::get("Computer") + L' ' + std::to_wstring(computers.size() + 1));
    broadcastLobbyState();
}

void HostLobbyScreen::setPublicIPAddressText() {
    if (!publicIPText.getString().isEmpty()) return;
    publicIPText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    publicIPText.setCharacterSize(20);
    #ifdef NDEBUG
    auto ip = sf::IpAddress::getPublicAddress(sf::seconds(5));
    #else
    auto ip = sf::IpAddress::None;
    #endif
    std::wstring wstr;
    if (ip != sf::IpAddress::None) {
        std::string str = ip.toString();
        wstr = std::wstring(str.begin(), str.end());
    } else wstr = Localisator::get("Cannot resolve IP");
    publicIPText.setString(Localisator::get("Your public IP address") + L" : " + wstr);
    publicIPText.setPosition(localIPText.getPosition().x, localIPText.getPosition().y + 30);
}

void HostLobbyScreen::clearPlayers() {
    for (auto const& client : clients) selector.remove(client->socket);
    clients.clear();
    resizeBgNames();
    broadcastLobbyState();
}

void HostLobbyScreen::clearComputers() {
    computers.clear();
    resizeBgNames();
    broadcastLobbyState();
}

void HostLobbyScreen::broadcastLobbyState() {
    sf::Packet packet;
    packet << LobbyCommand::UpdateLobby;

    uint16_t n = clients.size() + computers.size() + 1;
    packet << n;
    packet << namerhsText.getString().toWideString();
    for (auto const& client : clients) packet << client->name;
    for (auto const& name : computers) packet << name;

    for (auto const& client : clients) {
        auto status = client->socket.send(packet);
        if (status != sf::Socket::Done) {
            std::string name(client->name.begin(), client->name.end());
            std::cerr << "Could not send lobby names to client " << name << std::endl;
        }
    }
}

void HostLobbyScreen::broadcastHostName() {
    sf::Packet packet;
    packet << LobbyCommand::UpdateHostName;
    packet << namerhsText.getString().toWideString();
    for (auto const& client : clients) {
        auto status = client->socket.send(packet);
        if (status != sf::Socket::Done) {
            std::string name(client->name.begin(), client->name.end());
            std::cerr << "Could not send host name to client " << name << std::endl;
        }
    }
}

std::unique_ptr<Screen> HostLobbyScreen::startOnlineGame() {
    // si plus d'un client
    if (!clients.empty()) {
        std::vector<std::unique_ptr<Player>> players;
        for (auto& client : clients) players.push_back(std::move(client)); // clients
        for (auto const& name : computers) players.emplace_back(std::make_unique<Ai>(name)); // computers
        // host
        players.emplace_back(std::make_unique<Player>());
        players.back()->name =
                namerhsText.getString().toWideString();
        std::shuffle(players.begin(), players.end(), RandomEngine::instance());
        return std::make_unique<HostGameScreen>(window_, std::move(players));
    }
    return nullptr;
}
