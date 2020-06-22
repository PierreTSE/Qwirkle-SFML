#include "Engine/Cursor.hpp"

Cursor::Cursor() {
    cursor.loadFromSystem(sf::Cursor::Type::Arrow);
}

bool Cursor::setType(sf::Cursor::Type type) {
    if (type != currentType) {
        currentType = type;
        cursor.loadFromSystem(currentType);
        return true;
    }
    return false;
}

bool Cursor::setType(uint8_t type) {
    return setType(static_cast<sf::Cursor::Type>(type));
}
