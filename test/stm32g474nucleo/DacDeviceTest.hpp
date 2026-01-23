#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/DacDevice_DAC_DMA.hpp>
#include <coco/platform/opamp.hpp>
#include <coco/platform/timer.hpp>
#include <coco/board/config.hpp>


using namespace coco;


//using Sample = int8_t;
//constexpr auto MODE = dac::Config::CH1;
//constexpr auto FORMAT = dac::Format::RES_8;
//constexpr auto OFFSET = 127.0;
//constexpr auto AMPLITUDE = 126.0;
struct Sample {int16_t x; int16_t y;};
constexpr auto MODE = dac::Config::DUAL;
constexpr auto FORMAT = dac::Format::RES_12_LEFT;
constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;

constexpr int SAMPLE_COUNT = 1024;


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


// drivers for DacBufferTest
struct Drivers {
	Loop_TIM2 loop{APB1_TIMER_CLOCK};

	using Dac = DacDevice_DAC_DMA;
	Dac dac{loop,
		dacPins,
		//dac::DAC1_INFO,
		dac::DAC3_INFO,
		dma::DMA1_CH1_INFO,
		AHB_CLOCK,
		//MODE | dac::Config::BUFFERED_EXTERNAL, // DAC1 directly goes to pins
		MODE | dac::Config::INTERNAL, // DAC3 is internally connected to op-amps
		FORMAT,
		dac::Trigger::DAC1_TIM3_TRGO};
	Dac::Buffer1<2 * SAMPLE_COUNT * sizeof(Sample)> buffer1{dac};
	Dac::Buffer2 buffer2{buffer1};

	Drivers() {
		// enable opamps for DAC3
		opamp::configure(OPAMP3, opamp::Config::OUT_PIN | opamp::Config::OPAMP3_INP_DAC3_CH2 | opamp::Config::INM_FOLLOWER);
		opamp::configure(OPAMP6, opamp::Config::OUT_PIN | opamp::Config::OPAMP6_INP_DAC3_CH1 | opamp::Config::INM_FOLLOWER);

		// start DAC trigger timer
		timer::TIM3_INFO.configure(APB2_TIMER_CLOCK, 10kHz).setMaster(timer::MasterMode::UPDATE).start();
	}
};

Drivers drivers;

extern "C" {
void DMA1_Channel1_IRQHandler() {
	drivers.dac.DMA_IRQHandler();
}
}
