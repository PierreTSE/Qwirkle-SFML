#include "TitleScreen.hpp"
#include "../GameScreen.hpp"
#include "Localisator.hpp"
#include "RessourceLoader.hpp"
#include "Utilities.hpp"

TitleScreen::TitleScreen(sf::RenderWindow& window) : Screen(window) {
    bg.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    bg.setFillColor({53, 101, 77});

    qwirkle.setTexture(RessourceLoader::getTexture("sprites/Qwirkle.png"));
    centerOrigin(qwirkle);
    qwirkle.setPosition(static_cast<float>(window_.getSize().x) / 2, qwirkle.getGlobalBounds().height);

    sfml.setFont(RessourceLoader::getFont("fonts/CornFed.ttf"));
    sfml.setString(Localisator::get("with SFML"));
    sfml.setCharacterSize(30);
    centerOrigin(sfml);

    playerText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    playerText.setCharacterSize(30);
    setPlayerText();
    aiText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    aiText.setCharacterSize(30);
    setAiText();
    startText.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    startText.setCharacterSize(30);
    startText.setString(Localisator::get("Start"));
    centerOrigin(startText);

    cursor.setPointCount(3);
    cursor.setRadius(10);
    centerOrigin(cursor);

    adapt_viewport(window_);
}

void TitleScreen::adapt_viewport(sf::RenderWindow& window) {
    window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
    bg.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    qwirkle.setPosition(static_cast<float>(window_.getSize().x) / 2, qwirkle.getGlobalBounds().height);
    sfml.setPosition(qwirkle.getPosition() + sf::Vector2f{0, qwirkle.getGlobalBounds().height / 2 + sfml.getGlobalBounds().height});
    playerText.setPosition(sfml.getPosition() + sf::Vector2f{0, 200});
    aiText.setPosition(playerText.getPosition() + sf::Vector2f{0, 50});
    startText.setPosition(aiText.getPosition() + sf::Vector2f{0, 50});
}

std::unique_ptr<Screen> TitleScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {
                case sf::Event::MouseButtonReleased:
                case sf::Event::KeyReleased: {
                    switch (event.key.code) {
                        case sf::Keyboard::Up: if (cursorPos > 0) cursorPos -= 1;
                            break;
                        case sf::Keyboard::Down: if (cursorPos < 2) cursorPos += 1;
                            break;
                        case sf::Keyboard::Right:
                            if (cursorPos == 0 && players < 4) {
                                players += 1;
                                ai = std::min(ai, 4 - players);
                            } else if (cursorPos == 1 && ai < 4) {
                                ai += 1;
                                players = std::min(players, 4 - ai);
                            }
                            break;
                        case sf::Keyboard::Left:
                            if (cursorPos == 0 && players > 0) players -= 1;
                            else if (cursorPos == 1 && ai > 0) ai -= 1;
                            break;
                        case sf::Keyboard::Enter:
                            if (cursorPos == 2 && players + ai > 0) {
                                std::vector<PlayerType> v;
                                for (size_t i = 0; i < players; ++i) v.emplace_back(HUMAN);
                                for (size_t i = 0; i < ai; ++i) v.emplace_back(AI);
                                std::shuffle(v.begin(), v.end(), RandomEngine::instance());
                                return std::unique_ptr<Screen>(new GameScreen(window_, v));
                            }
                            break;
                    }
                    setPlayerText();
                    setAiText();
                }
            }
        }

        window_.clear();
        window_.draw(bg);
        window_.draw(qwirkle);
        window_.draw(sfml);
        window_.draw(playerText);
        window_.draw(aiText);
        window_.draw(startText);
        updateAndDrawCursor();
        window_.display();
        sf::sleep(sf::milliseconds(100));
    }
    return nullptr;
}

void TitleScreen::setPlayerText() {
    playerText.setString(Localisator::get("Players") + L" : " + std::to_wstring(players));
    centerOrigin(playerText);
}

void TitleScreen::setAiText() {
    aiText.setString(Localisator::get("Computers") + L" : " + std::to_wstring(ai));
    centerOrigin(aiText);
}

void TitleScreen::updateAndDrawCursor() {
    switch (cursorPos) {
        case 0:
            cursor.setPosition(
                    playerText.getPosition() + sf::Vector2f{-(playerText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width), 9});
            break;
        case 1:
            cursor.setPosition(
                    aiText.getPosition() + sf::Vector2f{-(aiText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width), 7});
            break;
        case 2:
            cursor.setPosition(
                    startText.getPosition() + sf::Vector2f{-(startText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width), 10});
            break;
    }
    cursor.setRotation(-90);
    window_.draw(cursor);
    switch (cursorPos) {
        case 0:
            cursor.setPosition(
                    playerText.getPosition() + sf::Vector2f{playerText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width, 7});
            break;
        case 1:
            cursor.setPosition(
                    aiText.getPosition() + sf::Vector2f{aiText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width, 5});
            break;
        case 2:
            cursor.setPosition(
                    startText.getPosition() + sf::Vector2f{startText.getGlobalBounds().width / 2 + cursor.getGlobalBounds().width, 8});
            break;
    }
    cursor.setRotation(90);
    window_.draw(cursor);
}
