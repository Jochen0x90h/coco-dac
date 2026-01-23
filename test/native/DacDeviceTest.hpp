#pragma once

#include <coco/platform/Loop_native.hpp>
#include <coco/platform/BufferDevice_cout.hpp>


using namespace coco;


using Sample = int8_t;
constexpr auto OFFSET = 127.5;
constexpr auto AMPLITUDE = 127.5;

constexpr int SAMPLE_COUNT = 1024;


// drivers for DacBufferTest
struct Drivers {
	Loop_native loop;

	using Dac = BufferDevice_cout;
	Dac dac{loop, "dac", 1s};
	Dac::Buffer buffer1{SAMPLE_COUNT, dac};
	Dac::Buffer buffer2{SAMPLE_COUNT, dac};
};

Drivers drivers;
