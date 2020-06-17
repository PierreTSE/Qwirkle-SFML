#include "Online/Lobby/WaitingLobbyScreen.hpp"
#include "Engine/Localisator.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Settings.hpp"
#include "Online/Commands.hpp"
#include "Online/ClientGameScreen.hpp"
#include "Online/Lobby/JoinLobbyScreen.hpp"
#include <iostream>


WaitingLobbyScreen::WaitingLobbyScreen(sf::RenderWindow& window, std::unique_ptr<sf::TcpSocket> hostSocket) :
        Screen{window},
        hostSocket{std::move(hostSocket)} {
    if (auto setting = Settings::get("onlinePort")) port = std::get<int>(setting.value());
    selector.add(*WaitingLobbyScreen::hostSocket);

    bgNames.setFillColor({0, 0, 0, 128});
    bgNames.setPosition(50, 140);

    playersText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    playersText.setCharacterSize(20);
    playersText.setStyle(sf::Text::Style::Underlined);
    playersText.setString(Localisator::get("Players"));

    nameText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    nameText.setCharacterSize(20);

    waitingText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    waitingText.setCharacterSize(20);
    waitingText.setString(Localisator::get("Waiting for start"));
    waitingText.setPosition(50, 50);

    waitingText.setPosition(50, 50);
    playersText.setPosition(waitingText.getPosition().x, waitingText.getPosition().y + 30);
}

void WaitingLobbyScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
}

std::unique_ptr<Screen> WaitingLobbyScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {
                default:;
            }
        }

        // drawing
        window_.clear();
        window_.draw(bg_);
        window_.draw(waitingText);
        drawPlayerNames();
        window_.display();

        // online management
        if (selector.wait(sf::milliseconds(50))) {
            if (selector.isReady(*hostSocket)) {
                sf::Packet packet;
                auto code = hostSocket->receive(packet);
                if (code == sf::Socket::Done) {
                    LobbyCommand command;
                    packet >> command;
                    switch (command) {
                        case LobbyCommand::UpdateHostName: {
                            std::wstring wstr;
                            packet >> wstr;
                            names.front() = wstr + (wstr.empty() ? L"" : L" ") + L'[' + Localisator::get("Host") + L']';
                        }
                            break;
                        case LobbyCommand::UpdateLobby: {
                            uint16_t n;
                            packet >> n;
                            names.clear();
                            for (size_t i = 0; i < n; ++i) {
                                std::wstring wstr;
                                packet >> wstr;
                                if (i == 0) wstr = wstr + (wstr.empty() ? L"" : L" ") + L'[' + Localisator::get("Host") + L']';
                                names.emplace_back(wstr);
                            }
                            resizeBGHost();
                        }
                            break;
                        case LobbyCommand::StartGame: {
                            return std::make_unique<ClientGameScreen>(window_, std::move(hostSocket), std::move(packet));
                        }
                    }
                } else if (code == sf::Socket::Disconnected) {
                    std::cerr << "You have been disconnected from the host" << std::endl;
                    return std::make_unique<JoinLobbyScreen>(window_);
                }
            }
        }

        sf::sleep(sf::milliseconds(50));
    }
    return nullptr;
}

void WaitingLobbyScreen::resizeBGHost() {
    bgNames.setSize({static_cast<float>(window_.getSize().x) - 100, names.size() * 30.f});
    bgNames.setPosition(50, 110);
    bgNames.setSize({static_cast<float>(window_.getSize().x) - 100, names.size() * 30.f});
}

void WaitingLobbyScreen::drawPlayerNames() {
    window_.draw(playersText);
    window_.draw(bgNames);
    size_t pos = 0;
    for (auto const& name : names) {
        nameText.setString(name);
        nameText.setPosition(80, playersText.getPosition().y + 30.f * ++pos);
        window_.draw(nameText);
    }
}
