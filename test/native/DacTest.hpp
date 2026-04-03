#pragma once

#include <coco/platform/Loop_native.hpp>
#include <coco/platform/Dac_cout.hpp>


using namespace coco;


// drivers for DacTest
struct Drivers {
    Loop_native loop;

    Dac_cout dac{"dac"};
};

Drivers drivers;
