#include "TitleScreen.hpp"
#include "GameScreen.hpp"
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

    leftCursor.setPointCount(3);
    leftCursor.setRadius(10);
    centerOrigin(leftCursor);
    rightCursor = leftCursor;
    leftCursor.setRotation(-90);
    rightCursor.setRotation(90);

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
                case sf::Event::KeyPressed: {
                    switch (event.key.code) {
                        case sf::Keyboard::Up:
                            if (cursorPos > 0) cursorPos -= 1;
                            else cursorPos = 3;
                            break;
                        case sf::Keyboard::Down:
                            if (cursorPos < 2) cursorPos += 1;
                            else cursorPos = 0;
                            break;
                        case sf::Keyboard::Right:addParticipant();
                            break;
                        case sf::Keyboard::Left:removeParticipant();
                            break;
                        case sf::Keyboard::Enter:if (auto gamescreen = switchToGameScreen()) return gamescreen;
                            break;
                    }
                }
                    break;
                case sf::Event::MouseButtonPressed : {
                    const auto posint = sf::Mouse::getPosition(window_);
                    const sf::Vector2f pos = {static_cast<float>(posint.x), static_cast<float>(posint.y)};
                    if (playerText.getGlobalBounds().contains(pos)) cursorPos = 0;
                    else if (aiText.getGlobalBounds().contains(pos)) cursorPos = 1;
                    else if (startText.getGlobalBounds().contains(pos)) {
                        if (cursorPos == 2) { if (auto gamescreen = switchToGameScreen()) return gamescreen; }
                        else cursorPos = 2;
                    } else if (leftCursor.getGlobalBounds().contains(pos)) removeParticipant();
                    else if (rightCursor.getGlobalBounds().contains(pos)) addParticipant();
                }
                    break;
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
            leftCursor.setPosition(
                    playerText.getPosition() + sf::Vector2f{-(playerText.getGlobalBounds().width / 2 + leftCursor.getGlobalBounds().width), 9});
            break;
        case 1:
            leftCursor.setPosition(
                    aiText.getPosition() + sf::Vector2f{-(aiText.getGlobalBounds().width / 2 + leftCursor.getGlobalBounds().width), 7});
            break;
        case 2:
            leftCursor.setPosition(
                    startText.getPosition() + sf::Vector2f{-(startText.getGlobalBounds().width / 2 + leftCursor.getGlobalBounds().width), 10});
            break;
    }
    window_.draw(leftCursor);
    switch (cursorPos) {
        case 0:
            rightCursor.setPosition(
                    playerText.getPosition() + sf::Vector2f{playerText.getGlobalBounds().width / 2 + rightCursor.getGlobalBounds().width, 5});
            break;
        case 1:
            rightCursor.setPosition(
                    aiText.getPosition() + sf::Vector2f{aiText.getGlobalBounds().width / 2 + rightCursor.getGlobalBounds().width, 5});
            break;
        case 2:
            rightCursor.setPosition(
                    startText.getPosition() + sf::Vector2f{startText.getGlobalBounds().width / 2 + rightCursor.getGlobalBounds().width, 8});
            break;
    }
    window_.draw(rightCursor);
}

void TitleScreen::addParticipant() {
    if (cursorPos == 0 && players < 4) {
        players += 1;
        ai = std::min(ai, 4 - players);
    } else if (cursorPos == 1 && ai < 4) {
        ai += 1;
        players = std::min(players, 4 - ai);
    }
    setPlayerText();
    setAiText();
}

void TitleScreen::removeParticipant() {
    if (cursorPos == 0 && players > 0) players -= 1;
    else if (cursorPos == 1 && ai > 0) ai -= 1;
    setPlayerText();
    setAiText();
}

std::unique_ptr<Screen> TitleScreen::switchToGameScreen() {
    if (cursorPos == 2 && players + ai > 0) {
        std::vector<PlayerType> v;
        for (size_t i = 0; i < players; ++i) v.emplace_back(HUMAN);
        for (size_t i = 0; i < ai; ++i) v.emplace_back(AI);
        std::shuffle(v.begin(), v.end(), RandomEngine::instance());
        return std::unique_ptr<Screen>(new GameScreen(window_, v));
    }
    return nullptr;
}
