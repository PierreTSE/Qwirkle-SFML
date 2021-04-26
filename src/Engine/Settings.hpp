#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>


class Settings {
public:
    Settings(Settings const&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(Settings const&) = delete;
    Settings& operator=(Settings&&) = delete;

    static std::optional<std::variant<bool, int, float, std::string>> get(std::string const& key);

private:
    Settings();
    static Settings& instance();

    std::unordered_map<std::string, std::variant<bool, int, float, std::string>> settings;
};


#endif //SCHMOUTSE_ENGINE_SETTINGS_HPP
