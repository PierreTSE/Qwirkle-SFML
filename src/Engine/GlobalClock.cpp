#include "GlobalClock.hpp"
#include <algorithm>


sf::Time GlobalClock::lap() noexcept {
    sf::Time elapsed = getInstance().clock.restart();

    if (getInstance().running) {
        getInstance().frame = elapsed * (float) getInstance().factor;
        getInstance().global += getInstance().frame;
        for (auto&[time, fun] : getInstance().callbacks) {
            time -= getInstance().frame;
            if (time < sf::Time::Zero)
                fun();
        }
        getInstance().callbacks.erase(std::remove_if(getInstance().callbacks.begin(),
                                                     getInstance().callbacks.end(),
                                                     [](auto const& p) { return p.first < sf::Time::Zero; }),
                                      getInstance().callbacks.end());
    } else {
        getInstance().frame = sf::Time::Zero;
    }

    return lapTime();
}

sf::Time GlobalClock::lapTime() noexcept {
    return getInstance().frame;
}

void GlobalClock::stop() noexcept {
    getInstance().running = false;
}

void GlobalClock::start() noexcept {
    getInstance().running = true;
}

void GlobalClock::changeSpeed(double speed) noexcept {
    getInstance().factor = speed;
}

sf::Time GlobalClock::timeSinceStartup() noexcept {
    return getInstance().global;
}

void GlobalClock::setTimeout(sf::Time delay, std::function<void()> fun) {
    getInstance().callbacks.emplace_back(delay, std::move(fun));
}

GlobalClock& GlobalClock::getInstance() {
    static GlobalClock instance;
    return instance;
}
