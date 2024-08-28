#include "GuidGenerator.h"

#include <random>
#include <sstream>
#include <iomanip>

std::string GuidGenerator::generate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20)
            ss << "-";
        ss << std::hex << std::setw(1) << dis(gen);
    }

    return ss.str();
}
