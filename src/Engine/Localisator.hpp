#ifndef LOCALISATOR_HPP
#define LOCALISATOR_HPP

#include <unordered_map>
#include <string>


class Localisator {
public:
    Localisator(Localisator const&) = delete;
    Localisator(Localisator&&) = delete;
    Localisator& operator=(Localisator const&) = delete;
    Localisator& operator=(Localisator&&) = delete;

    static std::wstring get(std::string const& key);

    static std::string getLocaleName();

private:
    Localisator();
    static Localisator& instance();

    std::string localeName;
    std::unordered_map<std::string, std::wstring> translations;
};


#endif //INCLUDE_LOCALISATOR_HPP
