#include "Engine/Settings.hpp"
#include "Engine/RessourceLoader.hpp"
#include "Engine/Utilities.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

Settings::Settings() {
    std::ifstream config(RessourceLoader::getPath("config.txt"));
    if (!config.is_open())
        std::clog << "Can't find config file (rc/config.txt)." << std::endl;

    std::string line;
    while (std::getline(config, line)) {
        if (line.empty() || line.at(0) == '#') continue;
        auto v = split(line, " = ");
        if (v.size() != 2)
            throw std::runtime_error("Error in config file : " + line);
        std::variant<bool, int, float, std::string> var;
        std::string str = v[1].substr(1);
        char c = v[1][0];
        switch (c) {
            case 'b': {
                std::istringstream is(str);
                bool b;
                is >> std::boolalpha >> b;
                var = b;
            }
                break;
            case 'i':var = std::stoi(str);
                break;
            case 'f':var = std::stof(str);
                break;
            case 's':var = str;
                break;
            default:throw std::runtime_error("Error in config file : " + line);
        }
        settings.emplace(v[0], var);
    }
    config.close();
}

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

std::optional<std::variant<bool, int, float, std::string>> Settings::get(const std::string& key) {
    auto it = instance().settings.find(key);
    if (it == instance().settings.end()) return std::nullopt;
    return it->second;
}