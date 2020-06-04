#ifndef GLOBALCLOCK_HPP
#define GLOBALCLOCK_HPP

#include <SFML/System/Clock.hpp>
#include <functional>
#include <vector>


class GlobalClock {
public:
    GlobalClock(GlobalClock const&) = delete;
    GlobalClock(GlobalClock&&) = delete;
    GlobalClock& operator=(GlobalClock const&) = delete;
    GlobalClock& operator=(GlobalClock&&) = delete;

    /**
     * mets à jour le temps de la frame, à appeler une fois dans la boucle de jeu principale
     */
    static sf::Time lap() noexcept;
    /**
     * temps d'une frame à appeler pour les calculs (anciennement : elapsedTime passé en argument)
     */
    static sf::Time lapTime() noexcept;
    static void stop() noexcept;
    static void start() noexcept;
    static void changeSpeed(double speed) noexcept;
    static sf::Time timeSinceStartup() noexcept;

    static void setTimeout(sf::Time delay, std::function<void()> fun);


private:
    GlobalClock() = default;

    static GlobalClock& getInstance();

    sf::Clock clock;
    sf::Time frame = sf::Time::Zero;
    sf::Time global = sf::Time::Zero;
    double factor = 1;
    bool running = true;
    std::vector<std::pair<sf::Time, std::function<void()>>> callbacks;
};

#endif // GLOBALCLOCK_HPP