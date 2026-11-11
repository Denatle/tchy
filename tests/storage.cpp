#include <cmath>
#include <iostream>
#include <touch/data/samples.hpp>
#include <vector>
using touch::Samples;
int main() {
    try {
        std::vector<float> values = {0,  1,      2,     3,       .1f,
                                     -2, -65504, 65504, .00001f, 0.000000059604645f};
        Samples buffer;
        buffer.assign(values.size(), 0);
        for (size_t i = 0; i < values.size(); ++i)
            buffer[i] = values[i];
        buffer.pack(1);
        const Samples& read = buffer;
        if (read.storage_bytes() != values.size() * 2)
            throw std::runtime_error("Half storage size");
        for (size_t i = 0; i < values.size(); ++i) {
#if defined(__clang__) || defined(__GNUC__)
            float expected = float((_Float16)values[i]);
#else
            float expected = values[i];
#endif
            if (std::abs(read[i] - expected) > std::max(1e-7f, std::abs(expected) * .001f))
                throw std::runtime_error("Half precision mismatch");
        }
        buffer.pack(2);
        if (buffer.storage_bytes() != values.size())
            throw std::runtime_error("Byte storage size");
        if (read[2] != 1 || read[5] != 0)
            throw std::runtime_error("UNORM bounds");
        auto copy = buffer;
        copy[0] = .75f;
        if (read[0] != 0)
            throw std::runtime_error("Copy mutates original");
        std::cout << "Packed f16/u8 storage checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
