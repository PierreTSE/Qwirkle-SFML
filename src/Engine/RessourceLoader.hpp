#ifndef RESSOURCELOADER_HPP
#define RESSOURCELOADER_HPP

#include <memory>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <map>
#include <SFML/Graphics/Font.hpp>


template<typename T>
struct RessourceReference {
    RessourceReference(T& t) : data{&t} {}

    RessourceReference(T* pt) : data{pt} {}

    RessourceReference& operator=(T& t) {
        data = &t;
        return *this;
    }

    RessourceReference& operator=(T* pt) {
        data = pt;
        return *this;
    }

    operator T&() { return *data; }

    operator T*() { return data; }

    T* operator->() { return data; }

    T* data = nullptr;
};

class RessourceLoader {
public:
    RessourceLoader(RessourceLoader const&) = delete;
    RessourceLoader(RessourceLoader&&) = delete;
    RessourceLoader& operator=(RessourceLoader const&) = delete;
    RessourceLoader& operator=(RessourceLoader&&) = delete;

    static RessourceReference<sf::Font> getFont(std::string const& name);
    static RessourceReference<sf::Texture> getTexture(std::string const& name);
    static RessourceReference<sf::SoundBuffer> getSoundBuffer(std::string const& name);
    static std::string getPath(std::string const& name);
    static RessourceLoader& getInstance();

private:
    RessourceLoader() = default;

    RessourceReference<sf::Texture> loadTexture(std::string const& name);
    RessourceReference<sf::Font> loadFont(std::string const& name);
    RessourceReference<sf::SoundBuffer> loadSoundBuffer(std::string const& name);

    bool wdSet = false;

    std::map<std::string, std::unique_ptr<sf::Font>> loadedFonts;
    std::map<std::string, std::unique_ptr<sf::Texture>> loadedTextures;
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> loadedSoundBuffers;
};


#endif // RESSOURCELOADER_HPP