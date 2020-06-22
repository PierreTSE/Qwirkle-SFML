#include "JoinLobbyScreen.hpp"
#include "Engine/Localisator.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Settings.hpp"
#include "Engine/Utilities.hpp"
#include "Online/Lobby/DefaultLobbyScreen.hpp"
#include <iostream>


JoinLobbyScreen::JoinLobbyScreen(sf::RenderWindow& window) : Screen(window) {
    if (auto setting = Settings::get("onlinePort")) port = std::get<int>(setting.value());
    hostSocket = std::make_unique<sf::TcpSocket>();

    cursor.setPointCount(3);
    cursor.setRadius(10);
    centerOrigin(cursor);
    cursor.setRotation(90);
    window_.setMouseCursor(mouseCursor);

    namelhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    namelhsText.setCharacterSize(20);
    namelhsText.setString(Localisator::get("Your name") + L" : ");
    namerhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    namerhsText.setCharacterSize(20);
    if (auto setting = Settings::get("name")) namerhsText.setString(std::get<std::string>(setting.value()));
    hostIPlhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    hostIPlhsText.setCharacterSize(20);
    hostIPlhsText.setString(Localisator::get("Host IP") + L" : ");
    hostIPrhsText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    hostIPrhsText.setCharacterSize(20);
    if (auto setting = Settings::get("preferredHostIP")) hostIPrhsText.setString(std::get<std::string>(setting.value()));
    joinText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    joinText.setCharacterSize(20);
    joinText.setString(Localisator::get("Join"));

    namelhsText.setPosition(50, 50);
    namerhsText.setPosition(namelhsText.getPosition().x + namelhsText.getGlobalBounds().width, namelhsText.getPosition().y);
    hostIPlhsText.setPosition(namelhsText.getPosition().x, namelhsText.getPosition().y + 30);
    hostIPrhsText.setPosition(hostIPlhsText.getPosition().x + hostIPlhsText.getGlobalBounds().width, hostIPlhsText.getPosition().y);
    joinText.setPosition(hostIPlhsText.getPosition().x, hostIPlhsText.getPosition().y + 30);
}

void JoinLobbyScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
}

std::unique_ptr<Screen> JoinLobbyScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {
                case sf::Event::MouseMoved: {
                    const sf::Vector2f pos = {static_cast<float>(sf::Mouse::getPosition(window_).x),
                                              static_cast<float>(sf::Mouse::getPosition(window_).y)};
                    if (namelhsText.getGlobalBounds().contains(pos) ||
                        hostIPlhsText.getGlobalBounds().contains(pos) ||
                        joinText.getGlobalBounds().contains(pos)) {
                        if (mouseCursor.setType(sf::Cursor::Type::Hand)) window_.setMouseCursor(mouseCursor);
                    } else if (mouseCursor.setType(sf::Cursor::Type::Arrow)) window_.setMouseCursor(mouseCursor);
                }
                    break;

                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Up:
                            if (cursorPos > 0) cursorPos -= 1;
                            else cursorPos = 2;
                            break;
                        case sf::Keyboard::Down:
                        case sf::Keyboard::Tab:
                            if (cursorPos < 2) cursorPos += 1;
                            else cursorPos = 0;
                            break;
                        case sf::Keyboard::Enter:
                            if (cursorPos == 2) { if (auto nextscreen = joinHost()) return nextscreen; }
                            break;
                        case sf::Keyboard::BackSpace:
                            if (cursorPos == 0 && !namerhsText.getString().isEmpty()) {
                                std::string str = namerhsText.getString();
                                str.pop_back();
                                namerhsText.setString(std::move(str));
                            } else if (cursorPos == 1 && !hostIPrhsText.getString().isEmpty()) {
                                std::string str = hostIPrhsText.getString();
                                str.pop_back();
                                hostIPrhsText.setString(std::move(str));
                            }
                            break;
                        case sf::Keyboard::V: {
                            if (event.key.control) {
                                std::wstring wstr = sf::Clipboard::getString();
                                switch (cursorPos) {
                                    case 0:
                                        namerhsText.setString(namerhsText.getString() + wstr);
                                        if (namerhsText.getString().getSize() > 50)
                                            namerhsText.setString(namerhsText.getString().substring(0, 50));
                                        break;
                                    case 1:
                                        hostIPrhsText.setString(hostIPrhsText.getString() + wstr);
                                        if (hostIPrhsText.getString().getSize() > 15)
                                            hostIPrhsText.setString(hostIPrhsText.getString().substring(0, 15));
                                        break;
                                }
                            }
                        }
                            break;
                        case sf::Keyboard::Escape:
                            return std::make_unique<DefaultLobbyScreen>(window_);
                    }
                    break;

                case sf::Event::MouseButtonPressed : {
                    const auto posint = sf::Mouse::getPosition(window_);
                    const sf::Vector2f pos = {static_cast<float>(posint.x), static_cast<float>(posint.y)};
                    if (namelhsText.getGlobalBounds().contains(pos) || namerhsText.getGlobalBounds().contains(pos)) {
                        cursorPos = 0;
                    } else if (hostIPlhsText.getGlobalBounds().contains(pos) || hostIPrhsText.getGlobalBounds().contains(pos)) {
                        cursorPos = 1;
                    } else if (joinText.getGlobalBounds().contains(pos)) {
                        if (cursorPos == 2) { if (auto nextscreen = joinHost()) return nextscreen; }
                        else cursorPos = 2;
                    }
                }
                    break;

                case sf::Event::TextEntered :
                    if (event.text.unicode < 32) break; // control chars
                    if (cursorPos == 0 && namerhsText.getString().getSize() < 50) {
                        namerhsText.setString(namerhsText.getString().toWideString() + static_cast<wchar_t>(event.text.unicode));
                    } else if (cursorPos == 1 && event.text.unicode < 128 && hostIPrhsText.getString().getSize() < 15) {
                        char c = static_cast<char>(event.text.unicode);
                        if (c == '.' || std::isdigit(c)) {
                            hostIPrhsText.setString(hostIPrhsText.getString() + c);
                        }
                    }
                    break;
            }
        }

        // drawing
        window_.clear();
        window_.draw(bg_);
        window_.draw(namerhsText);
        window_.draw(namelhsText);
        window_.draw(hostIPrhsText);
        window_.draw(hostIPlhsText);
        window_.draw(joinText);
        updateAndDrawCursor();
        window_.display();

        sf::sleep(sf::milliseconds(50));
    }
    return nullptr;
}

void JoinLobbyScreen::updateAndDrawCursor() {
    switch (cursorPos) {
        case 0:
            cursor.setPosition(namelhsText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -namelhsText.getGlobalBounds().height / 2});
            break;
        case 1:
            cursor.setPosition(hostIPlhsText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -hostIPlhsText.getGlobalBounds().height / 2});
            break;
        case 2:
            cursor.setPosition(joinText.getPosition() -
                               sf::Vector2f{cursor.getGlobalBounds().width, -joinText.getGlobalBounds().height / 2});
            break;
    }
    window_.draw(cursor);
}

std::unique_ptr<WaitingLobbyScreen> JoinLobbyScreen::joinHost() {
    if (hostIPrhsText.getString().isEmpty()) return nullptr;

    auto ip = sf::IpAddress(hostIPrhsText.getString().toAnsiString());
    if (ip == sf::IpAddress::None) {
        std::cout << "IP address not valid : " << hostIPrhsText.getString().toAnsiString() << std::endl;
        return nullptr;
    }

    if (namerhsText.getString().isEmpty()) {
        std::cout << "You must have a name" << std::endl;
        return nullptr;
    }

    if (hostSocket->connect(ip, port) == sf::Socket::Done) {
        sf::Packet packet;
        packet << namerhsText.getString().toWideString();
        if (hostSocket->send(packet) != sf::Socket::Done) {
            std::cerr << "Could not send name to host" << std::endl;
            return nullptr;
        }
        return std::make_unique<WaitingLobbyScreen>(window_, std::move(hostSocket));
    } else {
        std::cerr << "Could not connect to " << ip.toString() << std::endl;
    }
    return nullptr;
}

