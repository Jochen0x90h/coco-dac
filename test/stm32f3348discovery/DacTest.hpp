#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/board/config.hpp>


using namespace coco;


// DAC1 pins
const gpio::Config dacPins[] = {
    gpio::PA4, // channel 1 (PA4)
    gpio::PA5 // channel 2 (PA5)
};


// drivers for DacTest
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using Dac = Dac_DAC;
    Dac dac{
        dac::DAC1_INFO,
        dacPins,
        dac::DualConfig::CH2_OUTPUT_ENABLE}; // DAC1 of STM32F3348 has buffer off for channel 1 and output enable for channel 2
};

Drivers drivers;
