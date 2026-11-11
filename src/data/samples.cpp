#include <algorithm>
#include <bit>
#include <cmath>
#include <touch/data/samples.hpp>
namespace touch {
namespace {
float half_to_float(uint16_t value) {
    uint32_t sign = uint32_t(value & 0x8000) << 16, exponent = (value >> 10) & 31,
             mantissa = value & 1023;
    if (exponent == 0) {
        if (!mantissa)
            return std::bit_cast<float>(sign);
        return std::copysign(std::ldexp(float(mantissa), -24), sign ? -1.f : 1.f);
    }
    if (exponent == 31)
        return std::bit_cast<float>(sign | 0x7f800000 | (mantissa << 13));
    return std::bit_cast<float>(sign | ((exponent + 112) << 23) | (mantissa << 13));
}
uint16_t float_to_half(float value) {
    if (!std::isfinite(value))
        value = 0;
    value = std::clamp(value, -65504.f, 65504.f);
    uint32_t bits = std::bit_cast<uint32_t>(value), sign = (bits >> 16) & 0x8000;
    int exp = int((bits >> 23) & 255) - 127;
    uint32_t mantissa = (bits & 0x7fffff) | 0x800000;
    if (exp < -25)
        return uint16_t(sign);
    int shift = exp < -14 ? -exp - 1 : 13;
    // Preserve IEEE rounding (nearest, ties to even), including subnormals.
    uint32_t truncated = mantissa >> shift, remainder = mantissa & ((1u << shift) - 1),
             halfway = 1u << (shift - 1);
    if (remainder > halfway || (remainder == halfway && (truncated & 1)))
        ++truncated;
    if (exp < -14)
        return uint16_t(sign | truncated);
    return uint16_t(sign | (uint32_t((exp + 14) << 10) + truncated));
}
} // namespace
float Samples::packed(size_t i) const {
    if (format == 1)
        return half_to_float(halves[i]);
    if (format == 2)
        return bytes[i] / 255.f;
    return floats[i];
}
const float* Samples::expand() const {
    if (format == 0)
        return floats.data();
    std::lock_guard lock(cache->mutex);
    if (cache->values.empty()) {
        cache->values.resize(size());
        for (size_t i = 0; i < size(); ++i)
            cache->values[i] = (*this)[i];
    }
    return cache->values.data();
}
void Samples::writable() {
    if (format != 0) {
        std::vector<float> decoded(size());
        for (size_t i = 0; i < size(); ++i)
            decoded[i] = static_cast<const Samples&>(*this)[i];
        floats = std::move(decoded);
        cache = std::make_shared<Expanded>();
    }
    format = 0;
    halves.clear();
    bytes.clear();
}
void Samples::pack(int target) {
    cache = std::make_shared<Expanded>();
    if (target == format)
        return;
    writable();
    if (target == 1) {
        halves.reserve(floats.size());
        for (float x : floats)
            halves.push_back(float_to_half(x));
    } else if (target == 2) {
        bytes.reserve(floats.size());
        for (float x : floats)
            bytes.push_back(uint8_t(std::lround(std::clamp(x, 0.f, 1.f) * 255)));
    } else
        return;
    format = target;
    std::vector<float>().swap(floats);
}
} // namespace touch
