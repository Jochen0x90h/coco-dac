#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/platform/opamp.hpp>
#include <coco/board/config.hpp>


using namespace coco;


// DAC1 pins, use oscilloscope to measure the output
/*const gpio::Config dacPins[] = {
    gpio::PA4, // channel 1 (CN8 3)
    gpio::PA5 // channel 2 (CN5 6) Note: green LED is connected to this pin, therefore debug::setGreen() etc. does not work
};*/

// DAC3 pins, use oscilloscope to measure the output
const gpio::Config dacPins[] = {
    gpio::PB11, // channel 1 (CN10 18)
    gpio::PB1 // channel 2 (CN10 24)
};


/// @brief Drivers for DacTest
/// Make sure the VREF jumper is at default position (1-2)
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using Dac = Dac_DAC;
    Dac dac{
        //dac::DAC1_INFO,
        dac::DAC3_INFO,
        dacPins,
        AHB_CLOCK,
        //dac::DualConfig::BUFFERED_EXTERNAL, // DAC1 directly goes to pins
        dac::DualConfig::INTERNAL}; // DAC3 is internally connected to op-amps

    Drivers() {
        // enable opamps for DAC3
        opamp::enable(OPAMP6, opamp::Config::OUT_PIN | opamp::Config::OPAMP6_INP_DAC3_CH1 | opamp::Config::INM_FOLLOWER);
        opamp::enable(OPAMP3, opamp::Config::OUT_PIN | opamp::Config::OPAMP3_INP_DAC3_CH2 | opamp::Config::INM_FOLLOWER);
    }
};

Drivers drivers;
