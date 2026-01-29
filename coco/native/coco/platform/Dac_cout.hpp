#include <coco/Dac.hpp>
#include <string>


namespace coco {

/// @brief Implementation of a DAC that simply writes the values to std::cout.
///
class Dac_cout : public Dac {
public:
    Dac_cout(const std::string &name);
    ~Dac_cout() override;

    void set(int channel, int value) override;

protected:
    std::string name;
};

} // namespace coco
