#pragma once

#include <coco/Dac.hpp>
#include <coco/Array.hpp>
#include <coco/enum.hpp>
#include <coco/Frequency.hpp>
#include <coco/platform/dac.hpp>
#include <coco/platform/gpio.hpp>


#ifdef HAVE_DAC

namespace coco {

/**
 * Digital/analog converter implementation for STM32.
 *
 * Reference manual:
 *   f3:
 *     https://www.st.com/resource/en/reference_manual/rm0364-stm32f334xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
 *       DAC: Section 14
 *   g4:
 *     https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
 *       DAC: Section 22
 * Resources:
 *   DACx
 */
class Dac_DAC : public Dac {
public:
    /**
     * Constructor for the dual channel ADC device.
     * @param analogPins analog pins
     * @param dacInfo info of DAC to use
     * @param clockConfig clock configuration
     * @param config configuration of DAC channels
     */
    Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_CLOCK_CONFIG
        dac::ClockConfig clockConfig,
#endif
        dac::Config config);

#ifdef HAVE_DAC_CLOCK_CONFIG
    /**
     * Convenience constructor with parameter for AHB clock frequency
     * @param analogPins analog pins
     * @param dacInfo info of DAC to use
     * @param ahbClock AHB clock frequency (gets converted to clockConfig)
     * @param config configuration of DAC channels
     */
    Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo, Hertz<> ahbClock, dac::Config config)
        : Dac_DAC(analogPins, dacInfo, dac::ClockConfig((ahbClock.value - 1) / 80000000 << DAC_MCR_HFSEL_Pos),
        config)
    {}
#endif

    ~Dac_DAC() override;

    void set(int channel, int value) override;

protected:

    // DAC
    DAC_TypeDef *dac;

    // data regsiters
    unsigned count;
    __IO uint32_t *DR[2];
};

} // namespace coco

#endif // HAVE_DAC
