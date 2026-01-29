#include "Dac_cout.hpp"
#include <iostream>


namespace coco {

Dac_cout::Dac_cout(const std::string &name)
    : name(name)
{
}

Dac_cout::~Dac_cout() {
}

void Dac_cout::set(int channel, int value) {
    std::cout << this->name << ' ' << channel << ": " << value << std::endl;
}

} // namespace coco
