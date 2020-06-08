#include "RessourceLoader.hpp"
#include "View/GameScreen.hpp"
#include "View/TitleScreen.hpp"
#include <iostream>

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (auto arg = std::string(argv[i]); arg == "--version" || arg == "-V") {
            std::cout << "Version 1.1.1" << std::endl;
            std::exit(EXIT_SUCCESS);
        }
    }

    // Création de la fenêtre du jeu
    sf::RenderWindow window(
            sf::VideoMode{1280, 720}, "Qwirkle", sf::Style::Default, sf::ContextSettings(0, 0, 8));

    // Icone
    sf::Image thumbnail;
    thumbnail.loadFromFile(RessourceLoader::getPath("sprites/2.png"));
    window.setIcon(thumbnail.getSize().x, thumbnail.getSize().y, thumbnail.getPixelsPtr());

    // Title screen
    std::unique_ptr<Screen> screen(new TitleScreen(window));

    while (screen) screen = screen->execute();
}
