#ifndef QWIRKLE_SFML_CLIENT_HPP
#define QWIRKLE_SFML_CLIENT_HPP

#include "../Controller/Player.hpp"
#include <SFML/Network.hpp>

struct Client : public Player {
    explicit Client() = default;

    ClientType type() const override;

    sf::TcpSocket socket;
};


#endif //QWIRKLE_SFML_CLIENT_HPP
