#pragma once


namespace coco {

/**
	Simple DAC interface for setting an output value
*/
class Dac {
public:
	virtual ~Dac() {};

	/**
		Set the output value of a channel
		@param channel channel to set
		@param value value to set, typically 0 - 65535 or -32768 - 32767 where actual resolution is platform dependent
	*/
	virtual void set(int channel, int value) = 0;
};

} // namespace coco
