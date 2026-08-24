#include "DacDevice_DAC_DMA.hpp"
#include <coco/bits.hpp>
//#include <coco/debug.hpp>


#ifdef HAVE_DAC

namespace coco {

DacDevice_DAC_DMA::DacDevice_DAC_DMA(Loop_Queue &loop, const dac::Info &dacInfo, const gpio::Config analogPin,
    const dma::Info<dma::Feature::CIRCULAR> &dmaInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
    Hertz<> ahbClock,
#endif
    int channel, dac::Config config, dac::Format format, dac::Trigger trigger)
    : BufferDevice(State::READY)
    , loop_(loop)
{
    // configure pins as analog
    gpio::enableAnalog(analogPin);

    // initialize the DMA channel
    auto &dmaChannel = dmaChannel_ = dmaInfo.enableClock<DmaChannel::MODE>();

    auto dac = dacInfo
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        .enableClock(ahbClock)
#else
        .enableClock()
#endif
        .enable(channel, config, trigger, dac::DmaRequest::ENABLE);

#ifdef HAVE_DAC_DUAL_MODE
    auto DR = channel == 0 ? &dac->DHR12R1 : &dac->DHR12R2;
#else
    auto DR = &dac->DHR12R1;
#endif
    auto sourceSize = format == dac::Format::RES_8 ? 0 : 1; // source is 8 or 16 bit
    dmaChannel
        .configure(sourceSize, dma::Source::INCREMENT)
        .setDestinationAddress(DR + int(format));

    dmaIrq_ = dmaInfo.irq;
    nvic::setPriority(dmaIrq_, nvic::Priority::MEDIUM); // interrupt gets enabled in first call to start()

    // map DMA to DAC
    dacInfo.map(dmaInfo, channel);
}

#ifdef HAVE_DAC_DUAL_MODE
DacDevice_DAC_DMA::DacDevice_DAC_DMA(Loop_Queue &loop, const dac::Info &dacInfo, Array<const gpio::Config> analogPins,
    const dma::Info<dma::Feature::CIRCULAR> &dmaInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
    Hertz<> ahbClock,
#endif
    dac::DualConfig config, dac::Format format, dac::Trigger trigger)
    : BufferDevice(State::READY)
    , loop_(loop)
{
    // configure pins as analog
    for (auto pin : analogPins) {
        gpio::enableAnalog(pin);
    }

    // initialize the DMA channel
    auto &dmaChannel = dmaChannel_ = dmaInfo.enableClock<DmaChannel::MODE>();

    // configure DAC
    auto dac = dacInfo
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        .enableClock(ahbClock)
#else
        .enableClock()
#endif
        .enable(config, trigger, dac::DmaRequest::ENABLE);

    auto sourceSize = format == dac::Format::RES_8 ? 1 : 2; // source is 16 or 32 bit
    dmaChannel
        .configure(sourceSize, dma::Source::INCREMENT)
        .setDestinationAddress(&dac->DHR12RD + int(format));
    //dmaChannel.sourceSize = format == dac::Format::RES_8 ? 1 : 2; // 16 or 32 bit

    dmaIrq_ = dmaInfo.irq;
    nvic::setPriority(dmaIrq_, nvic::Priority::MEDIUM); // interrupt gets enabled in first call to start()

    // map DMA to DAC
    dacInfo.map(dmaInfo, 0);
}
#endif

DacDevice_DAC_DMA::~DacDevice_DAC_DMA() {
}

int DacDevice_DAC_DMA::getBufferCount() {
    return bufferCount_;
}

DacDevice_DAC_DMA::BufferBase &DacDevice_DAC_DMA::getBuffer(int index) {
    return *buffers_[index];
}

void DacDevice_DAC_DMA::handle(dma::Status status) {
    int queue = queue_;

    // stop DMA if no more pending transfers
    if ((queue & 0xf0) == 0) {
        dmaChannel_.disable();
    }

    // clear interrupt flag
    dmaChannel_.clear(status);

    // end of transfer
    int bufferIndex = (status & dma::Status::HALF_TRANSFER) != 0 ? 0 : 1;
    BufferBase *buffer = buffers_[bufferIndex];
    if (buffer->id_ == (queue & 0x0f)) {
        // pop queue
        queue_ = queue >> 4;

        // hand over to event loop (which calls BufferBase::handle())
        loop_.push(*buffer);
    }
}

void DacDevice_DAC_DMA::start() {
    // always start with first buffer
    auto &buffer = *buffers_[0];

    volatile uint8_t *data = buffer.data_;
    int size = buffer.capacity_ * 2;

    // configure DMA
    dmaChannel_
        .setSourceAddress(data)
        .setSourceSize(size)
        .enable(dma::Config::HALF_TRANSFER_INTERRUPT
            | dma::Config::TRANSFER_COMPLETE_INTERRUPT);
}


// DacDevice_DAC_DMA::BufferBase

DacDevice_DAC_DMA::BufferBase::BufferBase(uint8_t *data, int capacity, DacDevice_DAC_DMA &device, int id)
    : coco::Buffer(data, capacity, BufferBase::State::READY), device_(device), id_(id)
{
    assert(device.bufferCount_ < 2);
    device.buffers_[device.bufferCount_++] = this;
}

DacDevice_DAC_DMA::BufferBase::~BufferBase() {
}

bool DacDevice_DAC_DMA::BufferBase::start() {
    if (state_ != State::READY || (op_ & Op::WRITE) == 0 || size_ == 0) {
        // starting a buffer when the state is BUSY is a bug
        assert(state_ != State::BUSY);
        return false;
    }

    auto &device = device_;

    nvic::disable(device.dmaIrq_);

    int queue = device.queue_;
    int i = (queue & 0x0f) == 0 ? 0 : 4;
    device.queue_ = queue | (id_ << i);

    // start if DMA is stopped
    if (!device.dmaChannel_.enabled())
        device.start();

    nvic::enable(device.dmaIrq_);

    // set state
    setBusy();

    return true;
}

bool DacDevice_DAC_DMA::BufferBase::cancel() {
    if (state_ != State::BUSY)
        return false;

    // not supported, always complete normally
    return true;
}

void DacDevice_DAC_DMA::BufferBase::onCompletion() {
    // always transfers full capacity
    setSuccess(capacity_);
    setReady();
}

} // namespace coco

#endif // HAVE_DAC
