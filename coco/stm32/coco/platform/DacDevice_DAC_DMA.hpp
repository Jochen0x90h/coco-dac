#pragma once

#include <coco/align.hpp>
#include <coco/BufferDevice.hpp>
#include <coco/Frequency.hpp>
#include <coco/platform/Loop_Queue.hpp>
#include <coco/platform/dac.hpp>
#include <coco/platform/dma.hpp>
#include <coco/platform/gpio.hpp>
#include <coco/platform/nvic.hpp>


#ifdef HAVE_DAC

namespace coco {

/// @brief Digital/analog converter implementation for STM32 with circular DMA transfer.
/// Uses circular DMA transfers, therefore only two buffers are supported.
/// The first must be an instance of Buffer1 and the second an instance of Buffer2. When calling start() on a buffer,
/// the transfer always starts with the first buffer.
///
/// Resources:
///   DAC
///   DMA
class DacDevice_DAC_DMA : public BufferDevice {
public:
    /// @brief Constructor for single channel DAC.
    /// @param loop Event loop
    /// @param analogPin Analog pin for the DAC output
    /// @param dacInfo Info of DAC to use
    /// @param dmaInfo Info of DMA channel to use
    /// @param ahbClock Frequency of AHB clock
    /// @param channel Channel index (0 for channel 1, 1 for channel 2)
    /// @param config Configuration of DAC channel
    /// @param format Data format (resolution and alignment)
    /// @param trigger Trigger
    DacDevice_DAC_DMA(Loop_Queue &loop, gpio::Config analogPin, const dac::Info &dacInfo,
        const dma::Info<dma::Feature::CIRCULAR> &dmaInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        Hertz<> ahbClock,
#endif
        int channel, dac::Config config, dac::Format format, dac::Trigger trigger);

#ifdef HAVE_DAC_DUAL_MODE
    /// @brief Constructor for the dual channel DAC.
    /// @param loop Event loop
    /// @param analogPins Analog pins for the DAC outputs
    /// @param dacInfo Info of DAC to use
    /// @param dmaInfo Info of DMA channel to use
    /// @param ahbClock Frequency of AHB clock
    /// @param config Configuration of both DAC channels
    /// @param format Data format (resolution and alignment)
    /// @param trigger Trigger
    DacDevice_DAC_DMA(Loop_Queue &loop, Array<const gpio::Config> analogPins, const dac::Info &dacInfo,
        const dma::Info<dma::Feature::CIRCULAR> &dmaInfo,
#ifdef HAVE_DAC_PARAMETER_AHB_CLOCK
        Hertz<> ahbClock,
#endif
        dac::DualConfig config, dac::Format format, dac::Trigger trigger);
#endif

    ~DacDevice_DAC_DMA() override;


    // internal buffer base class, derives from IntrusiveListNode for the list of active transfers and Loop_Queue::Handler to be notified from the event loop
    class BufferBase : public coco::Buffer, public IntrusiveListNode, public Loop_Queue::Handler {
        friend class DacDevice_DAC_DMA;
    public:
        /// @brief Constructor
        /// @param headerCapacity capacity of the header
        /// @param data data of the buffer
        /// @param capacity capacity of the buffer
        /// @param channel channel to attach to
        BufferBase(uint8_t *data, int capacity, DacDevice_DAC_DMA &device, int id);
        ~BufferBase() override;

        /// @brief Transfer the buffer contents to the DAC. The whole buffer gets transferred regardless of the current size.
        ///
        bool start(Op op) override;
        bool cancel() override;

    protected:
        void handle() override;

        DacDevice_DAC_DMA &device_;
        int id_;
    };

    template <int C>
    class Buffer2;

    /// @brief Buffer for transferring data to the DAC.
    /// Internally uses circular mode, therefore only one instance of Buffer1 and Buffer2 are supported.
    /// @tparam C capacity of buffer, must be aligned to the sample size
    template <int C>
    class Buffer1 : public BufferBase {
        friend class Buffer2<C>;
    public:
        Buffer1(DacDevice_DAC_DMA &device) : BufferBase(data_, C, device, 1) {}

    protected:
        // data for Buffer1 and Buffer2
        alignas(4) uint8_t data_[C * 2];
    };

    template <int C>
    class Buffer2 : public BufferBase {
    public:
        Buffer2(Buffer1<C> &buffer) : BufferBase(buffer.data_ + C, C, buffer.device_, 2) {}
    };


    // BufferDevice methods
    int getBufferCount();
    BufferBase &getBuffer(int index);

    /// @brief Interrupt handler.
    /// Call from interrupt handler for the DMA channel (e.g. DMA1_Channel1_IRQHandler())
    void DMA_IRQHandler() {
        auto status = dmaChannel_.status() & (dma::Status::HALF_TRANSFER | dma::Status::TRANSFER_COMPLETE);
        if (status != 0) {
            handle(status);
        }
    }

protected:
    void handle(dma::Status status);
    void start();

    Loop_Queue &loop_;

    // DMA
    using DmaChannel = dma::Channel<dma::Mode::MEMORY_TO_PERIPHERAL | dma::Mode::CIRCULAR | dma::Mode::SOURCE_DYNAMIC | dma::Mode::DESTINATION_WIDTH_32>;
    DmaChannel dmaChannel_;
    int dmaIrq_;

    // list of buffers
    BufferBase *buffers_[2];
    int bufferCount_ = 0;

    // queue of buffer id's of up to two active transfers
    std::atomic<int> queue_ = 0;
};

} // namespace coco

#endif // HAVE_DAC
