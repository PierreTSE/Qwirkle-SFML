#ifndef GRID_HPP
#define GRID_HPP

#include "../Controller/Tile.hpp"
#include <SFML/Graphics.hpp>

/**
 * Classe qui permet le dessin d'une grille de rectangles 2D
 */
class Grid : public sf::Transformable {
public:
    Grid(float tileWidth, float tileHeight, unsigned int lineNumber, unsigned int colNumber, float lineThickness);
    Grid(float tileWidth, float tileHeight, unsigned int lineNumber, unsigned int colNumber, float lineThickness,
         const sf::Color& color);

    void draw(sf::RenderWindow& window) const;

    void centerOrigin();
    void centerIn(sf::Vector2u const& windowSize);
    void scale(float fx, float fy);

    sf::Vector2i getTileCoordinates(sf::Vector2i const& mousePosition) const;

    void updateTilesPositions();

    std::vector<Tile> tiles;

private:
    float tileWidth_, tileHeight_; // height of a square

    unsigned int lineNumber_ = 20, colNumber_ = 10;

    float lineThickness_ = 1;

    sf::Color color_ = {0, 0, 0};

    float getXLength() const;
    float getYLength() const;
};


#endif //GRID_HPP
