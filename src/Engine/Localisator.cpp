#include "Localisator.hpp"
#include "RessourceLoader.hpp"
#include "Settings.hpp"
#include "Utilities.hpp"
#include <codecvt>
#include <fstream>

Localisator::Localisator() {
    auto localesDir = RessourceLoader::getPath("locales") + "/";
    if (auto setting = Settings::get("locale")) {
        localeName = std::get<std::string>(setting.value());
    } else {
        localeName = "default";
    }

    std::ifstream locale(localesDir + localeName + ".txt");
    if (!locale.is_open())
        throw std::runtime_error("Can't open locale file " + localeName + ".txt");

    std::string line;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    while (std::getline(locale, line)) {
        if (line.empty() || line.at(0) == '#') continue;
        auto v = split(line, " = ");
        if (v.size() != 2)
            throw std::runtime_error("Error in locale file " + localeName + ".txt : " + line);
        translations[v.at(0)] = converter.from_bytes(v.at(1));
    }
    locale.close();
}

Localisator& Localisator::instance() {
    static Localisator instance;
    return instance;
}

std::wstring Localisator::get(const std::string& key) {
    auto it = instance().translations.find(key);
    if (it == instance().translations.end())
        throw std::runtime_error("Localisator can't find key : " + key);
    return it->second;
}

std::string Localisator::getLocaleName() {
    return instance().localeName;
}