#include "Engine/Utilities.hpp"
#include <random>

std::mt19937_64& RandomEngine::instance() {
    #if defined(WIN32) && (defined(__GNUC__) || defined(__GNUG__)) && !defined(_GLIBCXX_USE_DEV_RANDOM)
    static std::mt19937_64 random_engine(std::time(nullptr));
    #else
    static std::mt19937_64 random_engine(std::random_device{}());
    #endif
    return random_engine;
}

sf::Vector2f normalize(const sf::Vector2f& source) {
    const float length = std::hypot(source.x, source.y);
    if (length != 0)
        return sf::Vector2f(source.x / length, source.y / length);
    else
        return source;
}

int random(int min, int max) {
    std::uniform_int_distribution<int> distrib(min, max);
    return distrib(RandomEngine::instance());
}

int random(int max) { return random(0, max); }

double random(double min, double max) {
    std::uniform_real_distribution<double> distrib(min, max);
    return distrib(RandomEngine::instance());
}

double random(double max) {
    return random(0., max);
}

sf::Color HSVtoRGB(int hue, float sat, float val, float alpha) {
    hue %= 360;
    while (hue < 0) hue += 360;

    sat = std::clamp(sat, 0.f, 1.f);
    val = std::clamp(val, 0.f, 1.f);

    const int h = hue / 60;
    const float f = float(hue) / 60 - h;
    const float p = val * (1.f - sat);
    const float q = val * (1.f - sat * f);
    const float t = val * (1.f - sat * (1 - f));

    switch (h) {
        default :
        case 0 :
        case 1 :return sf::Color(q * 255, val * 255, p * 255, alpha);
        case 2 :return sf::Color(p * 255, val * 255, t * 255, alpha);
        case 3 :return sf::Color(p * 255, q * 255, val * 255, alpha);
        case 4 :return sf::Color(t * 255, p * 255, val * 255, alpha);
        case 5 :return sf::Color(val * 255, p * 255, q * 255, alpha);
        case 6 :return sf::Color(val * 255, t * 255, p * 255, alpha);
    }
}

std::filesystem::path strip_root(const std::filesystem::path& p) {
    const std::filesystem::path& parent_path = p.parent_path();
    if (parent_path.empty() || parent_path.string() == "/")
        return std::filesystem::path();
    else
        return strip_root(parent_path) / p.filename();
}

std::vector<std::string> split(const std::string& str, const std::string& sep) {
    std::vector<std::string> out;
    size_t debut = 0;
    size_t fin = str.find(sep);
    while (debut <= str.length()) {
        out.push_back(str.substr(debut, fin - debut));
        debut = fin != std::string::npos ? fin + sep.length() : fin;
        fin = str.find(sep, debut);
    }
    return out;
}
