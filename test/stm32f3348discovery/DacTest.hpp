#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/board/config.hpp>


using namespace coco;


constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;


// DAC1 pins
const gpio::Config dacPins[] = {
	gpio::Config::PA4, // channel 1 (PA4)
	gpio::Config::PA5 // channel 2 (PA5)
};


// drivers for DacTest
struct Drivers {
	Loop_TIM2 loop{APB1_TIMER_CLOCK};

	using Dac = Dac_DAC;
	Dac dac{dacPins,
		dac::DAC1_INFO,
		dac::Config::DUAL | dac::Config::DAC1_CH2_OUTPUT_ENABLE};
};

Drivers drivers;
