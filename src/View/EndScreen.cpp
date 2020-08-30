#include "EndScreen.hpp"
#include "Engine/alphanum.hpp"
#include "Engine/Localisator.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Utilities.hpp"
#include "View/TitleScreen.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>


void EndScreen::construct() {
    // retire les marqueurs de coup
    grid.tiles.erase(std::remove_if(grid.tiles.begin(), grid.tiles.end(),
                                    [](auto const& e) { return e.shapeID == 6; }), grid.tiles.end());

    text.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    text.setCharacterSize(30);
    gameEnded.setFont(RessourceLoader::getFont("fonts/Ubuntu-R.ttf"));
    gameEnded.setCharacterSize(60);
    gameEnded.setString(Localisator::get("Game ended"));
    gameEnded.setPosition(static_cast<float>(window_.getSize().x) / 2, 100);
    centerOrigin(gameEnded);

    mouseLastPos = sf::Mouse::getPosition();
}

EndScreen::EndScreen(sf::RenderWindow& window, std::vector<std::unique_ptr<Player>> const& players, Grid grid) :
        Screen(window),
        grid{std::move(grid)} {
    // écriture des stats
    std::wofstream ofs("rc/stats", std::ios::app);
    if (!ofs.is_open()) std::cerr << "Cannot open file to write results." << std::endl;
    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::string strtime = std::ctime(&time);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    ofs << converter.from_bytes(strtime.substr(0, 24)) << ';';
    if (players.size() > 1) {
        if (players.front()->score != players.at(1)->score) {
            if (players.front()->type() == ClientType::Ai) ofs << "Computer";
            else ofs << "Human";
        } else ofs << "Draw";
    } else ofs << "Solo";

    for (auto const& player : players) {
        scores.emplace_back(player->name, player->score);
    }

    std::sort(scores.begin(), scores.end(),
              [](auto const& a, auto const& b) {
                  return (a.second != b.second ? a.second > b.second : alphanum_less<decltype(a.first)>{}(a.first, b.first));
              });

    for (auto const& score : scores) {
        ofs << ';' << score.first << ", " << score.second;
    }
    ofs << std::endl;

    ofs.close();

    construct();
}

EndScreen::EndScreen(sf::RenderWindow& window, std::vector<std::pair<std::wstring, uint16_t>> scores, Grid grid) :
        Screen{window},
        scores{std::move(scores)},
        grid{std::move(grid)} {
    construct();
}

void EndScreen::adapt_viewport(sf::RenderWindow& window) {
    Screen::adapt_viewport(window);
    gameEnded.setPosition(static_cast<float>(window_.getSize().x) / 2, gameEnded.getPosition().y);
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
                            return std::make_unique<TitleScreen>(window_);
                        case sf::Keyboard::Escape:
                            return nullptr;
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
                case sf::Event::MouseWheelMoved: {
                    auto scale = grid.getScale();
                    auto f = 1 + 0.05f * event.mouseWheel.delta;
                    grid.scale(f * scale.x, f * scale.y);
                    grid.centerOrigin();
                }
                    break;
            }
        }

        window_.clear();
        window_.draw(bg_);
        grid.draw(window_);
        window_.draw(gameEnded);
        text.setString(Localisator::get("Scores"));
        text.setPosition(static_cast<float>(window_.getSize().x) / 2, gameEnded.getPosition().y + 100);
        text.setStyle(sf::Text::Style::Underlined);
        centerOrigin(text);
        window_.draw(text);
        text.setStyle(sf::Text::Style::Regular);
        for (size_t i = 0; i < scores.size(); ++i) {
            text.setString(scores.at(i).first + L" : " + std::to_wstring(scores.at(i).second));
            text.setPosition(static_cast<float>(window_.getSize().x) / 2, gameEnded.getPosition().y + 150 + 50 * i);
            centerOrigin(text);
            window_.draw(text);
        }
        window_.display();
        sf::sleep(sf::milliseconds(10));
    }
    return nullptr;
}
