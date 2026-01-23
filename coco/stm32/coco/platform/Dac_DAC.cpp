#include "Dac_DAC.hpp"
#include <coco/bits.hpp>
#include <coco/debug.hpp>


#ifdef HAVE_DAC

namespace coco {

Dac_DAC::Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_CLOCK_CONFIG
    dac::ClockConfig clockConfig,
#endif
    dac::Config config)
{
    // configure pins as analog
    for (auto pin : analogPins) {
        gpio::configureAnalog(pin);
    }

    // initialize DAC
    auto dac = this->dac = dacInfo.dac;
    dacInfo.configure(
#ifdef HAVE_DAC_CLOCK_CONFIG
        clockConfig,
#endif
        config);

    // always use 12 bit left aligned, i.e. simulated 16 bit
#ifdef DAC_CR_EN2
    bool ch1 = (config & dac::Config::CH1) != 0;
    bool ch2 = (config & dac::Config::CH2) != 0;
    if (ch1 && ch2) {
        // dual channel
        this->DR[0] = &dac->DHR12L1;
        this->DR[1] = &dac->DHR12L2;
        this->count = 2;
    } else if (ch2) {
        // only second channel
        this->DR[0] = &dac->DHR12L2;
        this->count = 1;
    } else
#endif
    {
        // only first channel
        this->DR[0] = &dac->DHR12L1;
        this->count = 1;
    }
}

Dac_DAC::~Dac_DAC() {
}

void Dac_DAC::set(int channel, int value) {
    if (unsigned(channel) >= this->count)
        return;
    *this->DR[channel] = value;
}

} // namespace coco

#endif // HAVE_DAC
