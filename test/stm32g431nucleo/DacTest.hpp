#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/board/config.hpp>


using namespace coco;


constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;


// DAC1 pins
const gpio::Config dacPins[] = {
	gpio::Config::PA4, // channel 1 (CN8 3)
	gpio::Config::PA5 // channel 2 (CN5 6) Note: green LED is connected to this pin, therefore debug::setGreen() etc. does not work
};


// drivers for DacTest
struct Drivers {
	Loop_TIM2 loop{APB1_TIMER_CLOCK};

	using Dac = Dac_DAC;
	Dac dac{dacPins,
		dac::DAC1_INFO,
		AHB_CLOCK,
		dac::Config::DUAL | dac::Config::BUFFERED_EXTERNAL}; // DAC1 directly goes to pins
};

Drivers drivers;
