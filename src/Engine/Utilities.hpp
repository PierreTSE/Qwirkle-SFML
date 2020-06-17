#ifndef UTILITIES_H
#define UTILITIES_H

#include <random>
#include <SFML/Graphics.hpp>
#include <filesystem>

template<typename T>
void centerOrigin(T& t) { t.setOrigin(t.getLocalBounds().width / 2.0, t.getLocalBounds().height / 2.0); }

sf::Vector2f normalize(const sf::Vector2f& source);

sf::Color HSVtoRGB(int hue, float sat, float val, float alpha); // hue: 0-360°; sat: 0.f-1.f; val: 0.f-1.f

struct RandomEngine {
    RandomEngine() = delete;
    RandomEngine(RandomEngine const&) = delete;
    void operator=(RandomEngine const&) = delete;
    static std::mt19937_64& instance();
};

int random(int min, int max);
int random(int max);
double random(double min, double max);
double random(double max);

std::filesystem::path strip_root(const std::filesystem::path& p);

std::vector<std::string> split(const std::string& str, const std::string& sep = " ");

#endif // !UTILITIES_H
