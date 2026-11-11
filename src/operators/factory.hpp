#pragma once
#include <touch/operators/module.hpp>
namespace touch {
std::function<std::unique_ptr<Operator>()> builtin_factory(const Descriptor&, MediaFactory&);
}
