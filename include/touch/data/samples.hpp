#pragma once
#include <compare>
#include <cstdint>
#include <iterator>
#include <memory>
#include <mutex>
#include <vector>
namespace touch {
class Samples {
    std::vector<float> floats;
    struct Expanded {
        std::mutex mutex;
        std::vector<float> values;
    };
    mutable std::shared_ptr<Expanded> cache = std::make_shared<Expanded>();
    std::vector<uint16_t> halves;
    std::vector<uint8_t> bytes;
    int format = 0;
    const float* expand() const;
    void writable();
    float packed(size_t) const;

  public:
    class Iterator {
        const Samples* owner = nullptr;
        std::ptrdiff_t index = 0;

      public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = float;
        using difference_type = std::ptrdiff_t;
        using reference = float;
        using pointer = void;
        Iterator() = default;
        Iterator(const Samples* p, std::ptrdiff_t i) : owner(p), index(i) {}
        float operator*() const {
            return (*owner)[index];
        }
        float operator[](difference_type n) const {
            return (*owner)[index + n];
        }
        Iterator& operator++() {
            ++index;
            return *this;
        }
        Iterator operator++(int) {
            auto old = *this;
            ++index;
            return old;
        }
        Iterator& operator--() {
            --index;
            return *this;
        }
        Iterator operator--(int) {
            auto old = *this;
            --index;
            return old;
        }
        Iterator& operator+=(difference_type n) {
            index += n;
            return *this;
        }
        Iterator& operator-=(difference_type n) {
            index -= n;
            return *this;
        }
        friend Iterator operator+(Iterator i, difference_type n) {
            i += n;
            return i;
        }
        friend Iterator operator+(difference_type n, Iterator i) {
            return i + n;
        }
        friend Iterator operator-(Iterator i, difference_type n) {
            i -= n;
            return i;
        }
        friend difference_type operator-(Iterator a, Iterator b) {
            return a.index - b.index;
        }
        auto operator<=>(const Iterator&) const = default;
    };
    Samples() = default;
    Samples& operator=(std::initializer_list<float> values) {
        format = 0;
        halves.clear();
        bytes.clear();
        floats = values;
        return *this;
    }
    void assign(size_t count, float value) {
        format = 0;
        halves.clear();
        bytes.clear();
        floats.assign(count, value);
    }
    size_t size() const {
        return format == 1 ? halves.size() : format == 2 ? bytes.size() : floats.size();
    }
    bool empty() const {
        return size() == 0;
    }
    float operator[](size_t i) const {
        return format == 0 ? floats[i] : packed(i);
    }
    float& operator[](size_t i) {
        if (format != 0)
            writable();
        return floats[i];
    }
    Iterator begin() const {
        return {this, 0};
    }
    Iterator end() const {
        return {this, std::ptrdiff_t(size())};
    }
    auto begin() {
        if (format != 0)
            writable();
        return floats.begin();
    }
    auto end() {
        if (format != 0)
            writable();
        return floats.end();
    }
    const float* data() const {
        return format == 0 ? floats.data() : expand();
    }
    float* data() {
        if (format != 0)
            writable();
        return floats.data();
    }
    template <class It> void insert(std::vector<float>::iterator where, It first, It last) {
        writable();
        floats.insert(where, first, last);
    }
    void pack(int storage_format);
    size_t storage_bytes() const {
        return format == 1 ? halves.size() * 2 : format == 2 ? bytes.size() : floats.size() * 4;
    }
};
} // namespace touch
