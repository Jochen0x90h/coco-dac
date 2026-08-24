#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/board/config.hpp>


using namespace coco;


// DAC1 pins, use oscilloscope to measure the output
const gpio::Config dacPins[] = {
    gpio::PA4, // channel 1 (CN8 3)
    gpio::PA5 // channel 2 (CN5 6) Note: green LED is connected to this pin, therefore debug::setGreen() etc. does not work
};


/// @brief Drivers for DacTest
/// Make sure the VREF jumper is at default position (1-2)
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using Dac = Dac_DAC;
    Dac dac{
        dac::DAC1_INFO,
        dacPins,
        AHB_CLOCK,
        dac::DualConfig::BUFFERED_EXTERNAL}; // DAC1 directly goes to pins
};

Drivers drivers;
