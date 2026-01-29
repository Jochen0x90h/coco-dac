#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/DacDevice_DAC_DMA.hpp>
#include <coco/platform/timer.hpp>
#include <coco/board/config.hpp>


using namespace coco;


//using Sample = int8_t;
//constexpr auto FORMAT = dac::Format::RES_8;
//constexpr auto OFFSET = 127.0;
//constexpr auto AMPLITUDE = 126.0;
struct Sample {int16_t x; int16_t y;};
constexpr auto FORMAT = dac::Format::RES_12_LEFT;
constexpr auto OFFSET = 32000.0;
constexpr auto AMPLITUDE = 30000.0;

constexpr int SAMPLE_COUNT = 1024;


// DAC1 pins, use oscilloscope to measure the output
const gpio::Config dacPins[] = {
    //gpio::PA4, // is VCP_TX
    gpio::PA5 // channel 2 (CN5 6) Note: green LED is connected to this pin, therefore debug::setGreen() etc. does not work
};


/// @brief Drivers for DacDeviceTest
/// Make sure the VREF jumper is at default position (1-2)
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using Dac = DacDevice_DAC_DMA;
    Dac dac{loop,
        dacPins,
        dac::DAC1_INFO,
        dma::DMA1_CH6_INFO,
        AHB_CLOCK,
        dac::DualConfig::BUFFERED_EXTERNAL, // DAC1 directly goes to pins
        FORMAT,
        dac::Trigger::DAC1_TIM3_TRGO};
    Dac::Buffer1<SAMPLE_COUNT * sizeof(Sample)> buffer1{dac};
    Dac::Buffer2<SAMPLE_COUNT * sizeof(Sample)> buffer2{buffer1};

    Drivers() {
        // start DAC trigger timer
        timer::TIM3_INFO.enableClock()
            .setUpdateFrequency(APB1_TIMER_CLOCK, 10kHz)
            .setMasterMode(timer::MasterMode::UPDATE)
            .start();
    }
};

Drivers drivers;

extern "C" {
void DMA1_Channel6_IRQHandler() {
    drivers.dac.DMA_IRQHandler();
}
}
