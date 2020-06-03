#include "EndScreen.hpp"
#include "Localisator.hpp"
#include "RessourceLoader.hpp"
#include "Utilities.hpp"

EndScreen::EndScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> players, Grid grid) :
        Screen(window),
        players{std::move(players)},
        grid{std::move(grid)} {
    bg.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    bg.setFillColor({53, 101, 77});

    text.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    text.setCharacterSize(30);
    gameEnded.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    gameEnded.setCharacterSize(60);
    gameEnded.setString(Localisator::get("Game ended"));
    gameEnded.setPosition(static_cast<float>(window_.getSize().x) / 2, 300);
    centerOrigin(gameEnded);

    mouseLastPos = sf::Mouse::getPosition();
    grid.setScale(0.999, 0.999);
}

void EndScreen::adapt_viewport(sf::RenderWindow& window) {
    window.setView(sf::View(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y)));
    bg.setSize({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    gameEnded.setPosition(static_cast<float>(window_.getSize().x) / 2, 300);
}

std::unique_ptr<Screen> EndScreen::execute() {
    while (window_.isOpen()) {
        sf::Event event;
        while (window_.pollEvent(event)) {
            auto result = manageEvent(event);
            if (result) return std::move(*result);
            switch (event.type) {
                case sf::Event::KeyPressed: {
                    switch (event.key.code) {
                        case sf::Keyboard::C: {
                            grid.centerIn(window_.getSize());
                        }
                            break;
                        case sf::Keyboard::Enter:
                        case sf::Keyboard::Escape:return nullptr;
                    }
                }
                    break;
                case sf::Event::MouseMoved: {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        grid.move(static_cast<float>(event.mouseMove.x - mouseLastPos.x),
                                  static_cast<float>(event.mouseMove.y - mouseLastPos.y));
                        grid.updateTilesPositions();
                    }
                    mouseLastPos = {event.mouseMove.x, event.mouseMove.y};
                }
                    break;
            }
        }

        window_.clear();
        window_.draw(bg);
        grid.draw(window_);
        window_.draw(gameEnded);
        text.setString(Localisator::get("Scores"));
        text.setPosition(static_cast<float>(window_.getSize().x) / 2, 400);
        text.setStyle(sf::Text::Style::Underlined);
        centerOrigin(text);
        window_.draw(text);
        text.setStyle(sf::Text::Style::Regular);
        for (size_t i = 0; i < players.size(); ++i) {
            text.setString(players.at(i)->name + L" : " + std::to_wstring(players.at(i)->score));
            text.setPosition(static_cast<float>(window_.getSize().x) / 2, 450 + 50 * i);
            centerOrigin(text);
            window_.draw(text);
        }
        window_.display();
        sf::sleep(sf::milliseconds(10));
    }
    return nullptr;
}
