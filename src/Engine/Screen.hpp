#ifndef ECRAN_H
#define ECRAN_H

#include <optional>
#include <memory>
#include <SFML/Graphics.hpp>


class Screen {
public:
    explicit Screen(sf::RenderWindow& window);
    virtual ~Screen() = default;

    virtual std::unique_ptr<Screen> execute() = 0;
    virtual std::optional<std::unique_ptr<Screen>> manageEvent(const sf::Event& event);

protected:
    sf::RenderWindow& window_;
    sf::RectangleShape bg_;

    virtual void adapt_viewport(sf::RenderWindow& window);
};

#endif //ECRAN_H
