#include "DefaultLobbyScreen.hpp"
#include "Engine/Localisator.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Utilities.hpp"
#include "Online/Lobby/HostLobbyScreen.hpp"
#include "Online/Lobby/JoinLobbyScreen.hpp"
#include "View/TitleScreen.hpp"

DefaultLobbyScreen::DefaultLobbyScreen(sf::RenderWindow& window) : Screen(window) {
    cursor.setPointCount(3);
    cursor.setRadius(10);
    centerOrigin(cursor);
    cursor.setRotation(90);

    createLobbyText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    createLobbyText.setCharacterSize(30);
    createLobbyText.setString(Localisator::get("Create a lobby"));
    centerOrigin(createLobbyText);
    joinLobbyText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    joinLobbyText.setCharacterSize(30);
    joinLobbyText.setString(Localisator::get("Join a lobby"));
    centerOrigin(joinLobbyText);

    DefaultLobbyScreen::adapt_viewport(window);
}

void DefaultLobbyScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
    createLobbyText.setPosition(static_cast<float>(window_.getSize().x) / 2, static_cast<float>(window_.getSize().y) / 2 - 50);
    joinLobbyText.setPosition(createLobbyText.getPosition().x, createLobbyText.getPosition().y + 50);
}

std::unique_ptr<Screen> DefaultLobbyScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {
                case sf::Event::KeyPressed: {
                    switch (event.key.code) {
                        case sf::Keyboard::Up:
                            if (cursorPos > 0) cursorPos -= 1;
                            else cursorPos = 1;
                            break;
                        case sf::Keyboard::Down:
                            if (cursorPos < 1) cursorPos += 1;
                            else cursorPos = 0;
                            break;
                        case sf::Keyboard::Enter:
                            if (cursorPos == 0) { if (auto nextscreen = HostLobbyScreen::factory(window_)) return nextscreen; }
                            else if (cursorPos == 1) return std::make_unique<JoinLobbyScreen>(window_);
                            break;
                        case sf::Keyboard::Escape:
                            return std::make_unique<TitleScreen>(window_);
                    }
                }
                    break;

                case sf::Event::MouseButtonPressed : {
                    const auto posint = sf::Mouse::getPosition(window_);
                    const sf::Vector2f pos = {static_cast<float>(posint.x), static_cast<float>(posint.y)};
                    if (createLobbyText.getGlobalBounds().contains(pos)) {
                        if (cursorPos == 0) { if (auto nextscreen = HostLobbyScreen::factory(window_)) return nextscreen; }
                        else cursorPos = 0;
                    } else if (joinLobbyText.getGlobalBounds().contains(pos)) {
                        if (cursorPos == 1) return std::make_unique<JoinLobbyScreen>(window_);
                        else cursorPos = 1;
                    }
                }
                    break;
            }
        }

        // drawing
        window_.clear();
        window_.draw(bg_);
        window_.draw(createLobbyText);
        window_.draw(joinLobbyText);
        updateAndDrawCursor();
        window_.display();

        sf::sleep(sf::milliseconds(50));
    }

    return nullptr;
}

void DefaultLobbyScreen::updateAndDrawCursor() {
    switch (cursorPos) {
        case 0:
            cursor.setPosition(createLobbyText.getPosition() -
                               sf::Vector2f{createLobbyText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width, 0});
            break;
        case 1:
            cursor.setPosition(joinLobbyText.getPosition() -
                               sf::Vector2f{joinLobbyText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width, 0});
            break;
    }
    window_.draw(cursor);
}
