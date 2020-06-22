#ifndef QWIRKLE_SFML_CURSOR_HPP
#define QWIRKLE_SFML_CURSOR_HPP

#include <cstdint>
#include <SFML/Window/Cursor.hpp>

struct Cursor {
    Cursor();
    sf::Cursor cursor;
    sf::Cursor::Type currentType = sf::Cursor::Arrow;

    bool setType(sf::Cursor::Type type);

    bool setType(uint8_t type);

    operator sf::Cursor const&() const { return cursor; }
};

#endif //QWIRKLE_SFML_CURSOR_HPP
