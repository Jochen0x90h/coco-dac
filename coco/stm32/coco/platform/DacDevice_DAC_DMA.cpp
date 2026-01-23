#include "DacDevice_DAC_DMA.hpp"
#include <coco/bits.hpp>
#include <coco/debug.hpp>


#ifdef HAVE_DAC

namespace coco {

DacDevice_DAC_DMA::DacDevice_DAC_DMA(Loop_Queue &loop, Array<const gpio::Config> analogPins, const dac::Info &dacInfo, const dma::Info &dmaInfo,
#ifdef HAVE_DAC_CLOCK_CONFIG
    dac::ClockConfig clockConfig,
#endif
    dac::Config config, dac::Format format, dac::Trigger trigger)
    : BufferDevice(State::READY)
    , loop(loop)
{
    // enable clocks (note two cycles wait time until peripherals can be accessed, see STM32G4 reference manual section 7.2.17)
    dmaInfo.rcc.enableClock();

    // configure pins as analog
    for (auto pin : analogPins) {
        gpio::configureAnalog(pin);
    }

    // initialize DAC and DMA channel
    auto dac = this->dac = dacInfo.dac;
    this->dmaStatus = dmaInfo.status();
    auto channel = this->dmaChannel = dmaInfo.channel();

    dacInfo.configure(
#ifdef HAVE_DAC_CLOCK_CONFIG
        clockConfig,
#endif
        config, trigger, dac::InternalConfig::ENABLE_DMA);

    int mainChannel = 0;
#ifdef DAC_CR_EN2
    bool ch1 = (config & dac::Config::CH1) != 0;
    bool ch2 = (config & dac::Config::CH2) != 0;
    if (ch1 && ch2) {
        // dual channel
        channel.setPeripheralAddress(&dac->DHR12RD + int(format));
        this->dmaShift = format == dac::Format::RES_8 ? 1 : 2; // 16 or 32 bit
    } else if (ch2) {
        // only second channel
        mainChannel = 1;
        channel.setPeripheralAddress(&dac->DHR12R2 + int(format));
        this->dmaShift = format == dac::Format::RES_8 ? 0 : 1; // 8 or 16 bit
    } else
#endif
    {
        // only first channel
        channel.setPeripheralAddress(&dac->DHR12R1 + int(format));
        this->dmaShift = format == dac::Format::RES_8 ? 0 : 1; // 8 or 16 bit
    }

    this->dmaIrq = dmaInfo.irq;
    nvic::setPriority(this->dmaIrq, nvic::Priority::MEDIUM); // interrupt gets enabled in first call to start()

    // map DMA to DAC
    dacInfo.map(dmaInfo, mainChannel);
}

DacDevice_DAC_DMA::~DacDevice_DAC_DMA() {
}

//StateTasks<const Device::State, Device::Events> &DacDevice_DAC_DMA::getStateTasks() {
//	return makeConst(this->st);
//}

int DacDevice_DAC_DMA::getBufferCount() {
    return this->bufferCount;
}

DacDevice_DAC_DMA::BufferBase &DacDevice_DAC_DMA::getBuffer(int index) {
    return *this->buffers[index];
}

void DacDevice_DAC_DMA::DMA_IRQHandler() {
    auto flags = this->dmaStatus.get();

    // check if read DMA has completed
    if ((flags & dma::Status::Flags::HALF_TRANSFER) != 0) {
        // clear interrupt flag
        this->dmaStatus.clear(dma::Status::Flags::HALF_TRANSFER);

        // stop DMA and DAC if buffer for second half is not ready
        if (!this->buffers[1]->active) {
            this->dmaChannel.disable();
            //this->dac->CR = 0;
        }

        // end of transfer
        BufferBase *buffer = this->buffers[0];
        if (buffer->active) {
            buffer->active = false;
            this->loop.push(*buffer);
        }
    }
    if ((flags & dma::Status::Flags::TRANSFER_COMPLETE) != 0) {
        // clear interrupt flag
        this->dmaStatus.clear(dma::Status::Flags::TRANSFER_COMPLETE);

        // stop DMA and DAC if buffer for first half is not ready
        if (!this->buffers[0]->active) {
            this->dmaChannel.disable();
            //this->dac->CR = 0;
        }

        // end of transfer
        BufferBase *buffer = this->buffers[1];
        if (buffer->active) {
            buffer->active = false;
            this->loop.push(*buffer);
        }
    }
}


// BufferBase

DacDevice_DAC_DMA::BufferBase::BufferBase(uint8_t *data, int capacity, DacDevice_DAC_DMA &device)
    : coco::Buffer(data, capacity, BufferBase::State::READY), device(device)
{
    assert(device.bufferCount < 2);
    device.buffers[device.bufferCount++] = this;
}

DacDevice_DAC_DMA::BufferBase::~BufferBase() {
}

bool DacDevice_DAC_DMA::BufferBase::start(Op op) {
    if (this->st.state != State::READY) {
        assert(this->st.state != State::BUSY);
        return false;
    }

    // check if READ flag is set
    assert((op & Op::READ) != 0);

    auto &device = this->device;

    nvic::disable(device.dmaIrq);
    this->active = true;

    // start if DMA is stopped
    if (!device.dmaChannel.enabled())
        start();

    nvic::enable(device.dmaIrq);

    // set state
    setBusy();

    return true;
}

bool DacDevice_DAC_DMA::BufferBase::cancel() {
    if (this->st.state != State::BUSY)
        return false;

    // always complete normally
    return true;
}

void DacDevice_DAC_DMA::BufferBase::start() {
    auto &device = this->device;

    // always start first buffer
    auto &buffer = *device.buffers[0];

    int dmaShift = device.dmaShift;
    auto data = buffer.p.data;
    int count = (buffer.p.capacity >> dmaShift) * 2;

    // configure DMA
    device.dmaChannel.setMemoryAddress(data);
    device.dmaChannel.setCount(count);
    device.dmaChannel.enable(dma::Channel::Config::TX
        | dma::Channel::Config::PERIPHERAL_SIZE_32
        | dma::Channel::Config::HALF_TRANSFER_INTERRUPT
        | dma::Channel::Config::TRANSFER_COMPLETE_INTERRUPT
        | dma::Channel::Config::CIRCULAR,
        dmaShift);

    //device.dac->CR = device.CR;
}

void DacDevice_DAC_DMA::BufferBase::handle() {
    auto &buffer = *device.buffers[0];
    int dmaShift = device.dmaShift;
    int transferred = (buffer.p.capacity >> dmaShift) << dmaShift;

   setReady(transferred);
}

} // namespace coco

#endif // HAVE_DAC
