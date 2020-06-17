#include "View/Grid.hpp"

Grid::Grid(float tileWidth, float tileHeight, unsigned int lineNumber, unsigned int colNumber, float lineThickness)
        : tileWidth_(tileWidth),
          tileHeight_(tileHeight),
          lineNumber_(lineNumber),
          colNumber_(colNumber),
          lineThickness_(lineThickness) {}

Grid::Grid(float tileWidth, float tileHeight, unsigned int lineNumber, unsigned int colNumber, float lineThickness, const sf::Color& color)
        : Grid(tileWidth, tileHeight, lineNumber, colNumber, lineThickness) { color_ = color; }

void Grid::draw(sf::RenderWindow& window) const {
    auto scale = getScale();
    // horizontal
    sf::RectangleShape line({getXLength(), lineThickness_ * scale.y});
    line.setFillColor(color_);
    line.setOrigin(lineThickness_ / 2 * scale.x, lineThickness_ / 2 * scale.y);
    line.setPosition(getOrigin().x + getPosition().x, getOrigin().y + getPosition().y);
    for (size_t i = 0; i <= lineNumber_; ++i) {
        window.draw(line);
        line.move(0, (tileHeight_ + lineThickness_) * scale.y);
    }
    // vertical
    line.setSize({getYLength(), lineThickness_ * scale.y});
    line.setOrigin(lineThickness_ / 2 * scale.x, lineThickness_ / 2 * scale.y);
    line.setPosition(getOrigin().x + getPosition().x, getOrigin().y + getPosition().y);
    line.rotate(90);
    // line.setFillColor(sf::Color::Black);
    for (size_t i = 0; i <= colNumber_; ++i) {
        window.draw(line);
        line.move((tileWidth_ + lineThickness_) * scale.x, 0);
    }

    // tiles
    for (auto& tile : tiles) {
        if (!tile.disp) continue;
        window.draw(tile);
    }
}

float Grid::getXLength() const {
    return (colNumber_ * (tileWidth_ + lineThickness_) + lineThickness_) * getScale().x;
}

float Grid::getYLength() const {
    return (lineNumber_ * (tileHeight_ + lineThickness_) + lineThickness_) * getScale().y;
}

void Grid::centerOrigin() {
    setOrigin(-getXLength() / 2, -getYLength() / 2);
    updateTilesPositions();
}

sf::Vector2i Grid::getTileCoordinates(sf::Vector2i const& mousePosition) const {
    sf::Vector2f mouseLocalPos =
            sf::Vector2f{static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)} - getPosition() - getOrigin();
    mouseLocalPos.x /= getScale().x;
    mouseLocalPos.y /= getScale().y;

    const sf::Vector2f localSize = {getXLength() / getScale().x, getYLength() / getScale().y};

    // std::cout << mouseLocalPos.x << " " << mouseLocalPos.y << std::endl;

    if (mouseLocalPos.x <= lineThickness_ / 2 || mouseLocalPos.y <= lineThickness_ / 2)
        return {-1, -1};

    if (mouseLocalPos.x >= localSize.x - lineThickness_ / 2 || mouseLocalPos.y >= localSize.y - lineThickness_ * 1.5)
        return {-1, -1};

    mouseLocalPos.x -= lineThickness_ / 2;
    mouseLocalPos.y -= lineThickness_ / 2;

    int x = mouseLocalPos.x / (tileWidth_ + lineThickness_);
    if (mouseLocalPos.x - (x + 1) * (tileWidth_ + lineThickness_) + lineThickness_ > 0) x = -1;

    int y = mouseLocalPos.y / (tileHeight_ + lineThickness_);
    if (mouseLocalPos.y - (y + 1) * (tileHeight_ + lineThickness_) + lineThickness_ > 0) y = -1;

    return {x, y};
}

void Grid::scale(float fx, float fy) {
    setScale(fx, fy);
    for (auto& tile : tiles) {
        tile.setScale(fx, fy);
    }
}

void Grid::updateTilesPositions() {
    const float x = (lineThickness_ + tileWidth_) * getScale().x;
    const float y = (lineThickness_ + tileHeight_) * getScale().y;
    const float offsetx = x / 2 + getPosition().x + getOrigin().x;
    const float offsety = y / 2 + getPosition().y + getOrigin().y;
    for (auto& tile : tiles) {
        tile.setScale(getScale());
        tile.setPosition(tile.coord.x * x + offsetx, tile.coord.y * y + offsety);
    }
}

void Grid::centerIn(const sf::Vector2u& windowSize) {
    setPosition({static_cast<float>(windowSize.x) / 2, static_cast<float>(windowSize.y) / 2});
    updateTilesPositions();
}


