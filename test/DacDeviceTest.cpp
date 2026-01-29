#include <coco/debug.hpp>
#include <DacDeviceTest.hpp>
#ifdef NATIVE
#include <iostream>
#endif
#include <cmath>

// sample type is e.g. int16_t
void setSample(std::integral auto &sample, int i) {
    sample = int(OFFSET + AMPLITUDE * sin(i * 6.28318530718 / 128.0));
}

// sample type is a struct containing x and y
void setSample(auto &sample, int i) {
    sample.x = int(OFFSET + AMPLITUDE * sin(i * 6.28318530718 / 128.0));
    sample.y = int(OFFSET + AMPLITUDE * cos(i * 6.28318530718 / 128.0));
}

Coroutine write(Loop &loop, Buffer &buffer1, Buffer &buffer2) {
    Sample table[128];
    for (int i = 0; i < 128; ++i) {
        setSample(table[i], i);
    }

    int j = 0;
    while (true) {
        co_await buffer1.untilReadyOrDisabled();
        auto data1 = buffer1.pointer<Sample>();
        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            data1[i] = table[j & 127];
            ++j;
        }
#ifdef NATIVE
        std::cout << "start 1" << std::endl;
#else
        debug::toggleGreen();
#endif
        buffer1.startWrite(SAMPLE_COUNT * sizeof(Sample));

        co_await buffer2.untilReadyOrDisabled();
        auto data2 = buffer2.pointer<Sample>();
        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            data2[i] = table[j & 127];
            j += 1;
        }
#ifdef NATIVE
        std::cout << "start 2" << std::endl;
#else
        debug::toggleGreen();
#endif
        buffer2.startWrite(SAMPLE_COUNT * sizeof(Sample));

        //co_await loop.sleep(1s);
    }
}


int main() {
    debug::out << "DacDeviceTest\n";

    // write to dac
    write(drivers.loop, drivers.buffer1, drivers.buffer2);

    drivers.loop.run();
}
