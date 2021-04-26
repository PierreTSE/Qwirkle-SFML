#ifndef QWIRKLE_SFML_AIGAMECONTROLLER_HPP
#define QWIRKLE_SFML_AIGAMECONTROLLER_HPP


#include "Controller/Ai.hpp"
#include <vector>

class AIGameController {
public:

    std::vector<int> wins;

    explicit AIGameController(std::vector<ai::Strategy> const& strategies);
    static void parseArgs(int argc, char** argv);
    void simulateGame();


private:

    Controller controller;
    std::vector<ai::Strategy> strategies;

};


#endif //QWIRKLE_SFML_AIGAMECONTROLLER_HPP
