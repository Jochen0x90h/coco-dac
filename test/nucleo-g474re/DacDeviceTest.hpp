#pragma once

#include <coco/platform/Loop_TIM2.hpp>
#include <coco/platform/DacDevice_DAC_DMA.hpp>
#include <coco/platform/opamp.hpp>
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
/*const gpio::Config dacPins[] = {
    gpio::PA4, // channel 1 (CN8 3)
    gpio::PA5 // channel 2 (CN5 6) Note: green LED is connected to this pin, therefore debug::setGreen() etc. does not work
};*/

// DAC3 pins, use oscilloscope to measure the output
const gpio::Config dacPins[] = {
    gpio::PB11, // channel 1 (CN10 18)
    gpio::PB1 // channel 2 (CN10 24)
};


/// @brief Drivers for DacDeviceTest
/// Make sure the VREF jumper is at default position (1-2)
struct Drivers {
    Loop_TIM2 loop{APB1_TIMER_CLOCK};

    using Dac = DacDevice_DAC_DMA;
    Dac dac{loop,
        //dac::DAC1_INFO,
        dac::DAC3_INFO,
        dacPins,
        dma::DMA1_CH1_INFO,
        AHB_CLOCK,
        //dac::DualConfig::BUFFERED_EXTERNAL, // DAC1 directly goes to pins
        dac::DualConfig::INTERNAL, // DAC3 is internally connected to op-amps
        FORMAT,
        dac::Trigger::DAC1_TIM3_TRGO};
    Dac::Buffer1<SAMPLE_COUNT * sizeof(Sample)> buffer1{dac};
    Dac::Buffer2<SAMPLE_COUNT * sizeof(Sample)> buffer2{buffer1};

    Drivers() {
        // enable opamps for DAC3
        opamp::enable(OPAMP6, opamp::Config::OUT_PIN | opamp::Config::OPAMP6_INP_DAC3_CH1 | opamp::Config::INM_FOLLOWER);
        opamp::enable(OPAMP3, opamp::Config::OUT_PIN | opamp::Config::OPAMP3_INP_DAC3_CH2 | opamp::Config::INM_FOLLOWER);

        // start DAC trigger timer
        timer::TIM3_INFO.enableClock()
            .setUpdateFrequency(APB1_TIMER_CLOCK, 10kHz)
            .setMasterMode(timer::MasterMode::UPDATE)
            .start();
    }
};

Drivers drivers;

extern "C" {
void DMA1_Channel1_IRQHandler() {
    drivers.dac.DMA_IRQHandler();
}
}
