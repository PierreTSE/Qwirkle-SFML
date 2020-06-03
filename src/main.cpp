#include "GameScreen.hpp"
#include "RessourceLoader.hpp"
#include "View/TitleScreen.hpp"

int main() {
    // Création de la fenêtre du jeu
    sf::RenderWindow window(
            sf::VideoMode{1280, 720}, "Qwirkle", sf::Style::Default, sf::ContextSettings(0, 0, 8));

    // Icone
    sf::Image thumbnail;
    thumbnail.loadFromFile(RessourceLoader::getPath("sprites/2.png"));
    window.setIcon(thumbnail.getSize().x, thumbnail.getSize().y, thumbnail.getPixelsPtr());

    // Title screen
    // std::unique_ptr<Screen> screen(new TitleScreen(window));
    std::unique_ptr<Screen> screen(new GameScreen{window, {AI}});

    while (screen) screen = screen->execute();

    return 0;
}
