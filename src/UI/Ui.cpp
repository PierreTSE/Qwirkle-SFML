#include "Ui.hpp"
#include "RessourceLoader.hpp"
#include "Utilities.hpp"

Ui::Ui() {
    setSize({10 + 40 + 5 + 40 + 10, 5 + 40 + 5});
    setFillColor(sf::Color{128, 128, 128});

    recycleButton.setTexture(RessourceLoader::getTexture("sprites/bin.png"));
    centerOrigin(recycleButton);
    nextTurnButton.setTexture(RessourceLoader::getTexture("sprites/next.png"));
    centerOrigin(nextTurnButton);
}

void Ui::draw(sf::RenderWindow& window) const {
    // window.draw(*this);
    window.draw(recycleButton);
    window.draw(nextTurnButton);
}

void Ui::replace(sf::Vector2u const& windowSize) {
    setPosition(static_cast<float>(windowSize.x) / 2 + 137.5f, static_cast<float>(windowSize.y) - 102);
    recycleButton.setPosition(getPosition() + sf::Vector2f{10 + 20, 5 + 20});
    nextTurnButton.setPosition(getPosition() + sf::Vector2f{10 + 45 + 20, 5 + 20});
}

bool Ui::isOnRecycleButton(sf::Vector2i const& pos) {
    return std::hypot(pos.x - recycleButton.getPosition().x, pos.y - recycleButton.getPosition().y) < 20;
}

bool Ui::isOnNextTurnButton(sf::Vector2i const& pos) {
    return std::hypot(pos.x - nextTurnButton.getPosition().x, pos.y - nextTurnButton.getPosition().y) < 20;
}
