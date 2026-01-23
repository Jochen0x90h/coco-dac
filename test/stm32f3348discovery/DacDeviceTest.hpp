#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/DacDevice_DAC_DMA.hpp>
#include <coco/platform/timer.hpp>
#include <coco/board/config.hpp>


using namespace coco;


//using Sample = int8_t;
//constexpr auto MODE = dac::Config::CH1;
//constexpr auto FORMAT = dac::Format::RES8;
//constexpr auto OFFSET = 127.0;
//constexpr auto AMPLITUDE = 126.0;
struct Sample {int16_t x; int16_t y;};
constexpr auto MODE = dac::Config::DUAL;
constexpr auto FORMAT = dac::Format::RES_12_LEFT;
constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;

constexpr int SAMPLE_COUNT = 1024;


// DAC1 pins
const gpio::Config dacPins[] = {
	gpio::Config::PA4, // channel 1 (PA4)
	gpio::Config::PA5 // channel 2 (PA5)
};


// drivers for DacBufferTest
struct Drivers {
	Loop_TIM2 loop{APB1_TIMER_CLOCK};

	using Dac = DacDevice_DAC_DMA;
	Dac dac{loop,
		dacPins,
		dac::DAC1_INFO,
		dma::DMA1_CH3_INFO,
		MODE | dac::Config::DAC1_CH2_OUTPUT_ENABLE,
		FORMAT,
		dac::Trigger::DAC1_TIM6_TRGO};
	Dac::Buffer1<2 * SAMPLE_COUNT * sizeof(Sample)> buffer1{dac};
	Dac::Buffer2 buffer2{buffer1};

	Drivers() {
		// start DAC trigger timer
		timer::TIM6_INFO.configure(APB1_TIMER_CLOCK, 10kHz).setMaster(timer::MasterMode::UPDATE).start();
	}
};

Drivers drivers;

extern "C" {
void DMA1_Channel3_IRQHandler() {
	drivers.dac.DMA_IRQHandler();
}
}
