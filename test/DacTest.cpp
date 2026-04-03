#include <coco/debug.hpp>
#include <DacTest.hpp>
#include <cmath>
#ifdef NATIVE
#include <iostream>
#endif


Coroutine write(Loop &loop, Dac &dac) {
    uint8_t i = 0;
    while (true) {
        // set level to ADC inputs using ADC
        dac.set(0, i << 8);
        dac.set(1, int8_t(i + 128) << 8);
        ++i;

        co_await loop.sleep(2ms);
#ifndef NATIVE
        debug::set(i >> 4);
#endif
    }
}


int main() {
    debug::out << "DacTest\n";

    // write to dac
    write(drivers.loop, drivers.dac);

    drivers.loop.run();
}
