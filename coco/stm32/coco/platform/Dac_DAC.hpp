#pragma once

#include <coco/Dac.hpp>
#include <coco/Array.hpp>
#include <coco/enum.hpp>
#include <coco/Frequency.hpp>
#include <coco/platform/dac.hpp>
#include <coco/platform/gpio.hpp>


#ifdef HAVE_DAC

namespace coco {

/// @brief Digital/analog converter implementation for STM32.
///
/// Resources:
///   DAC
class Dac_DAC : public Dac {
public:

    /// @brief Constructor for single channel.
    /// @param analogPins Analog pins
    /// @param dacInfo Info of DAC to use
    /// @param ahbClock Frequency of AHB clock
    /// @param channel Channel index (0 for channel 1, 1 for channel 2)
    /// @param config Configuration of DAC channel
    Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        Hertz<> ahbClock,
#endif
        int channel, dac::Config config);

#ifdef HAVE_DAC_DUAL_MODE
    /// @brief Constructor for dual channel.
    /// @param analogPins Analog pins
    /// @param dacInfo Info of DAC to use
    /// @param ahbClock Frequency of AHB clock
    /// @param config Configuration of both DAC channels
    Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        Hertz<> ahbClock,
#endif
        dac::DualConfig config);
#endif

    ~Dac_DAC() override;

    void set(int channel, int value) override;

protected:

    // DAC
    DAC_TypeDef *dac_;

    // data regsiters
    unsigned count_;
    __IO uint32_t *DR_[2];
};

} // namespace coco

#endif // HAVE_DAC
