#pragma once

#include "Dac.hpp"


namespace coco {

/// @brief Dummy DAC interface implementation that does nothing
///
class DacDummy : public Dac {
public:
	~DacDummy() override;

	void set(int channel, int value) override;
};

} // namespace coco
