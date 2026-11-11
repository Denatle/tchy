#pragma once
#include "../common.hpp"
namespace touch::ops {
Value require(const std::vector<Value>& inputs, size_t index);
Results oscillator(const std::string&, Oscillator&, const Node&, const std::vector<Value>&,
                   const Context&,const Parameters&);
Results filter(const std::string&, Oscillator&, const std::vector<Value>&, const Parameters&);
Results mixer(const std::string&, const Node&, const std::vector<Value>&, const Parameters&);
Results playback(const std::string&, Oscillator&, const Node&, const std::vector<Value>&,
                 const Parameters&);
} // namespace touch::ops
