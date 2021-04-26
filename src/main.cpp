#include "Benchmark/AIGameController.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Settings.hpp"
#include "View/TitleScreen.hpp"
#include <iostream>


int main(int argc, char** argv) {
    const std::string appName = "Qwirkle-SFML "
                                + std::to_string(QWIRKLE_SFML_MAJOR) + "."
                                + std::to_string(QWIRKLE_SFML_MINOR) + "."
                                + std::to_string(QWIRKLE_SFML_PATCH);

    for (int i = 1; i < argc; ++i) {
        if (auto arg = std::string(argv[i]); arg == "--version" || arg == "-V") {
            std::cout << appName << std::endl;
            std::exit(EXIT_SUCCESS);
        } else if (arg == "--ai-benchmark") {
            AIGameController::parseArgs(argc, argv);
            std::exit(EXIT_SUCCESS);
        }
    }

    // Settings
    if (auto setting = Settings::get("antialiasing")) RessourceLoader::setSmooth(std::get<bool>(setting.value()));
    uint8_t antialiasingLevel = 8;
    if (auto setting = Settings::get("antialiasingLevel")) antialiasingLevel = std::get<int>(setting.value());


    // Création de la fenêtre de jeu
    sf::RenderWindow window(sf::VideoMode{1280, 720},
                            appName,
                            sf::Style::Default,
                            sf::ContextSettings(0, 0, antialiasingLevel));

    // Icone
    sf::Image thumbnail;
    thumbnail.loadFromFile(RessourceLoader::getPath("sprites/2.png"));
    window.setIcon(thumbnail.getSize().x, thumbnail.getSize().y, thumbnail.getPixelsPtr());

    // Title screen
    std::unique_ptr<Screen> screen = std::make_unique<TitleScreen>(window);

    while (screen) screen = screen->execute();
}