#include "Dac_DAC.hpp"
#include <coco/bits.hpp>
//#include <coco/debug.hpp>


#ifdef HAVE_DAC

namespace coco {

Dac_DAC::Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
    Hertz<> ahbClock,
#endif
    int channel, dac::Config config)
{
    // configure pins as analog
    for (auto pin : analogPins) {
        gpio::enableAnalog(pin);
    }

    // configure DAC
    auto dac = dac_ = dacInfo
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        .enableClock(ahbClock)
#else
        .enableClock()
#endif
        .enable(channel, config);

    // single channel
#ifdef HAVE_DAC_DUAL_MODE
    DR_[0] = channel == 0 ? &dac->DHR12L1 : &dac->DHR12L2;
#else
    DR_[0] = &dac->DHR12L1;
#endif
    count_ = 1;
}

#ifdef HAVE_DAC_DUAL_MODE
Dac_DAC::Dac_DAC(Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
    Hertz<> ahbClock,
#endif
    dac::DualConfig config)
{
    // configure pins as analog
    for (auto pin : analogPins) {
        gpio::enableAnalog(pin);
    }

    // configure DAC
    auto dac = dac_ = dacInfo
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        .enableClock(ahbClock)
#else
        .enableClock()
#endif
        .enable(config);

    // dual channel
    DR_[0] = &dac->DHR12L1;
    DR_[1] = &dac->DHR12L2;
    count_ = 2;
}
#endif

Dac_DAC::~Dac_DAC() {
}

void Dac_DAC::set(int channel, int value) {
    if (unsigned(channel) >= count_)
        return;
    *DR_[channel] = value;
}

} // namespace coco

#endif // HAVE_DAC
