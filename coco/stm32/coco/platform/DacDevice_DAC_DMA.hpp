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

/**
 * Digital/analog converter with DMA transfer. Uses circular DMA transfers, therefore only two buffers are supported.
 * The first must be an instance of Buffer1 and the second an instance of Buffer2. When calling start() on a buffer,
 * the transfer always starts with the first buffer.
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
 *   DMAx
 */
class DacDevice_DAC_DMA : public BufferDevice {
public:
    /**
     * Constructor for the dual ADC device.
     * @param loop event loop
     * @param analogPins analog pins
     * @param dacInfo info of DAC to use
     * @param dmaInfo info of DMA channel to use
     * @param clockConfig clock configuration
     * @param config configuration of DAC channels
     * @param trigger trigger, see reference manual
     */
    DacDevice_DAC_DMA(Loop_Queue &loop, Array<const gpio::Config> analogPins, const dac::Info &dacInfo, const dma::Info &dmaInfo,
#ifdef HAVE_DAC_CLOCK_CONFIG
        dac::ClockConfig clockConfig,
#endif
        dac::Config config, dac::Format format, dac::Trigger trigger);

#ifdef HAVE_DAC_CLOCK_CONFIG
    /**
     * Convenience constructor with parameter for AHB clock frequency
     * @param loop event loop
     * @param analogPins analog pins
     * @param dacInfo info of DAC to use
     * @param dmaInfo info of DMA channel to use
     * @param ahbClock AHB clock frequency (gets converted to clockConfig)
     * @param config configuration of DAC channels
     * @param trigger trigger, see reference manual
     */
    DacDevice_DAC_DMA(Loop_Queue &loop, Array<const gpio::Config> analogPins, const dac::Info &dacInfo, const dma::Info &dmaInfo,
        Hertz<> ahbClock, dac::Config config, dac::Format format, dac::Trigger trigger)
        : DacDevice_DAC_DMA(loop, analogPins, dacInfo, dmaInfo,
        dac::ClockConfig((ahbClock.value - 1) / 80000000 << DAC_MCR_HFSEL_Pos), config, format, trigger)
    {}
#endif

    ~DacDevice_DAC_DMA() override;

    class Buffer2;

    // internal buffer base class, derives from IntrusiveListNode for the list of active transfers and Loop_Queue::Handler to be notified from the event loop
    class BufferBase : public coco::Buffer, public IntrusiveListNode, public Loop_Queue::Handler {
        friend class DacDevice_DAC_DMA;
        friend class Buffer2;
    public:
        /**
         * Constructor
         * @param headerCapacity capacity of the header
         * @param data data of the buffer
         * @param capacity capacity of the buffer
         * @param channel channel to attach to
         */
        BufferBase(uint8_t *data, int capacity, DacDevice_DAC_DMA &device);
        ~BufferBase() override;

        /**
         * Transfer the buffer contents to the DAC. The whole buffer gets transferred regardless of the current size.
         */
        bool start(Op op) override;
        bool cancel() override;

    protected:
        void start();
        void handle() override;

        DacDevice_DAC_DMA &device;
        std::atomic<bool> active = false;
    };

    /**
     * Buffer for transferring data to the DAC.
     * Internally uses circular mode, therefore only one instance of Buffer1 and Buffer2 are allowed.
     * @tparam C capacity of buffer
     */
    template <int C>
    class Buffer1 : public BufferBase {
        friend class Buffer2;
    public:
        Buffer1(DacDevice_DAC_DMA &device) : BufferBase(data, C / 2, device) {}

    protected:
        alignas(4) uint8_t data[C];
    };

    class Buffer2 : public BufferBase {
    public:
        template <int C>
        Buffer2(Buffer1<C> &buffer) : BufferBase(buffer.data + C / 2, C / 2, buffer.device) {}
    };


    // Device methods
    //StateTasks<const State, Events> &getStateTasks() override;

    // BufferDevice methods
    int getBufferCount();
    BufferBase &getBuffer(int index);

    /**
     * Call from interrupt handler for the DMA channel (e.g. DMA1_Channel1_IRQHandler())
     */
    void DMA_IRQHandler();

protected:
    Loop_Queue &loop;

    // DAC
    DAC_TypeDef *dac;

    // DMA
    dma::Status dmaStatus;
    dma::Channel dmaChannel;
    int dmaIrq;
    int dmaShift;

    // device state
    //StateTasks<Device::State, Device::Events> st = Device::State::READY;

    // list of buffers
    BufferBase *buffers[2];
    int bufferCount = 0;
};

} // namespace coco

#endif // HAVE_DAC
