#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/Dac_DAC.hpp>
#include <coco/platform/opamp.hpp>
#include <coco/platform/vref.hpp>
#include <coco/board/config.hpp>


using namespace coco;


//constexpr auto OFFSET = 127.0;
//constexpr auto AMPLITUDE = 126.0;
constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;


// DAC1 pins
/*const gpio::Config dacPins[] = {
	gpio::Config::PA4, // channel 1 (CN8 3)
	gpio::Config::PA5 // channel 2 (CN5 6)
};*/

// DAC3 pins
const gpio::Config dacPins[] = {
	gpio::Config::PB11, // channel 1 (CN10 18)
	gpio::Config::PB1 // channel 2 (CN10 24)
};


// drivers for DacTest
struct Drivers {
	Loop_TIM2 loop{APB1_TIMER_CLOCK};

	using Dac = Dac_DAC;
	Dac dac{dacPins,
		//dac::DAC1_INFO,
		dac::DAC3_INFO,
		AHB_CLOCK,
		//dac::Config::UNSIGNED | dac::Config::BUFFERED_EXTERNAL, // DAC1 directly goes to pins
		dac::Config::DUAL | dac::Config::INTERNAL}; // DAC3 is internally connected to op-amps

	Drivers() {
		// enable opamps for DAC3
		opamp::configure(OPAMP3, opamp::Config::OUT_PIN | opamp::Config::OPAMP3_INP_DAC3_CH2 | opamp::Config::INM_FOLLOWER);
		opamp::configure(OPAMP6, opamp::Config::OUT_PIN | opamp::Config::OPAMP6_INP_DAC3_CH1 | opamp::Config::INM_FOLLOWER);
	}
};

Drivers drivers;
