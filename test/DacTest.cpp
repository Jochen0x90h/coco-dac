#include <coco/debug.hpp>
#include <DacTest.hpp>
#include <cmath>
#ifdef NATIVE
#include <iostream>
#endif


Coroutine write(Loop &loop, Dac &dac) {
    int i = 0;
    while (true) {
        dac.set(0, int(OFFSET + AMPLITUDE * sin((i & 127) * 6.28318530718 / 128.0)));
        dac.set(1, int(OFFSET + AMPLITUDE * cos((i & 127) * 6.28318530718 / 128.0)));
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
