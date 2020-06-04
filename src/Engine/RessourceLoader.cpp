#include "RessourceLoader.hpp"
#include <filesystem>


RessourceReference<sf::Font> RessourceLoader::getFont(std::string const& name) {
    if (RessourceLoader::getInstance().loadedFonts.find(name) != RessourceLoader::getInstance().loadedFonts.end())
        return RessourceLoader::getInstance().loadedFonts[name].get();
    else
        return RessourceLoader::getInstance().loadFont(name);
}

RessourceReference<sf::Texture> RessourceLoader::getTexture(std::string const& name) {
    if (RessourceLoader::getInstance().loadedTextures.find(name) != RessourceLoader::getInstance().loadedTextures.end())
        return RessourceLoader::getInstance().loadedTextures[name].get();
    else
        return RessourceLoader::getInstance().loadTexture(name);
}

RessourceReference<sf::SoundBuffer> RessourceLoader::getSoundBuffer(std::string const& name) {
    if (RessourceLoader::getInstance().loadedSoundBuffers.find(name) != RessourceLoader::getInstance().loadedSoundBuffers.end())
        return RessourceLoader::getInstance().loadedSoundBuffers[name].get();
    else
        return RessourceLoader::getInstance().loadSoundBuffer(name);
}

RessourceReference<sf::Texture> RessourceLoader::loadTexture(std::string const& name) {
    const std::string path = getPath(name);
    auto tex = std::make_unique<sf::Texture>();
    tex->loadFromFile(path);
    return loadedTextures.insert({name, std::move(tex)}).first->second.get();
}

RessourceReference<sf::Font> RessourceLoader::loadFont(std::string const& name) {
    const std::string path = getPath(name);
    auto font = std::make_unique<sf::Font>();
    font->loadFromFile(path);
    return loadedFonts.insert({name, std::move(font)}).first->second.get();
}

std::string RessourceLoader::getPath(std::string const& name) {
    if (!RessourceLoader::getInstance().wdSet) {
        using namespace std::filesystem;
        path p = current_path();
        while (!exists(p / "rc") && p.has_parent_path())
            p = p.parent_path();
        if (exists(p / "rc"))
            current_path(p);
        else
            throw std::runtime_error("Can't find resources directory.");
        RessourceLoader::getInstance().wdSet = true;
    }
    return "rc/" + name;
}

RessourceReference<sf::SoundBuffer> RessourceLoader::loadSoundBuffer(std::string const& name) {
    std::string path = getPath(name);
    auto snd = std::make_unique<sf::SoundBuffer>();
    snd->loadFromFile(path);
    return loadedSoundBuffers.insert({name, std::move(snd)}).first->second.get();
}

RessourceLoader& RessourceLoader::getInstance() {
    static RessourceLoader instance;
    return instance;
}