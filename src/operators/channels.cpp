#include "common.hpp"
namespace touch::ops {
Maybe channels(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
               const Parameters& p) {
    auto get = [&](size_t i) -> Value {
        if (i >= in.size() || !in[i])
            throw std::runtime_error("Connect input " + std::to_string(i + 1));
        return in[i];
    };
    auto choice = [&](int i) { return n.choice(i); };
    if (in.empty() || !in[0])
        return {};
    auto a = in[0];
    if (k == "ImageChannels" || k == "MaskChannels" || k == "AudioChannels") {
        int group = k == "ImageChannels" && choice(0) == 5 ? 4 : 1;
        int count = k == "AudioChannels" ? a->components : a->height * group;
        auto b = make(Signal::Channels, a->width, count);
        for (int row = 0; row < count; ++row) {
            if (k == "AudioChannels")
                b->names.push_back(size_t(row) < a->names.size() ? a->names[row]
                                                                 : "channel" + std::to_string(row));
            else {
                auto index = std::to_string(row / group);
                std::string prefix =
                    "row" + std::string(std::max(0, 4 - int(index.size())), '0') + index + ".";
                std::string suffix =
                    k == "MaskChannels" ? "mask"
                    : group == 4 ? std::vector<std::string>{"r", "g", "b", "a"}[row % 4]
                                 : std::vector<std::string>{"luma", "r", "g", "b", "a"}[choice(0)];
                b->names.push_back(prefix + suffix);
            }
            for (int x = 0; x < a->width; ++x) {
                float v;
                if (k == "AudioChannels")
                    v = a->values[x * a->components + row];
                else if (k == "MaskChannels")
                    v = a->values[row * a->width + x];
                else {
                    auto src = (a->values.data() + (((row / group) * a->width + x) * 4));
                    v = group == 4 ? src[row % 4] : choice(0) == 0 ? luma(src) : src[choice(0) - 1];
                }
                b->values[row * a->width + x] = v;
            }
        }
        return Results{b};
    }
    if (k == "ChannelsImage" || k == "ChannelsMask") {
        int group = k == "ChannelsImage" && choice(0) == 1 ? 4 : 1;
        if (a->height % group)
            throw std::runtime_error("RGBA requires groups of four channels");
        auto b = make(k == "ChannelsMask" ? Signal::Mask : Signal::Video, a->width,
                      a->height / group, k == "ChannelsMask" ? 1 : 4);
        for (int y = 0; y < b->height; ++y)
            for (int x = 0; x < b->width; ++x)
                for (int ch = 0; ch < b->components; ++ch)
                    b->values[(y * b->width + x) * b->components + ch] =
                        group == 4 ? a->values[(y * 4 + ch) * a->width + x]
                                   : (ch == 3 ? 1 : a->values[y * a->width + x]);
        if (k == "ChannelsMask")
            for (auto& value : b->values)
                value = clamp(value);
        else if (group == 4)
            for (size_t i = 3; i < b->values.size(); i += 4)
                b->values[i] = clamp(b->values[i]);
        return Results{b};
    }
    if (k == "SelectChannels") {
        std::vector<int> indices;
        if (n.names.empty()) {
            int first = int(std::lround(p(0))) - 1,
                last = p(1) == 0 ? a->height : std::min(a->height, first + int(std::lround(p(1))));
            for (int i = first; i < last; ++i)
                indices.push_back(i);
        } else {
            size_t start = 0;
            while (start < n.names.size()) {
                auto end = n.names.find(',', start);
                auto name = n.names.substr(start, end - start);
                auto f = name.find_first_not_of(" \t"), l = name.find_last_not_of(" \t");
                name = f == std::string::npos ? "" : name.substr(f, l - f + 1);
                auto it = std::find(a->names.begin(), a->names.end(), name);
                if (it == a->names.end())
                    throw std::runtime_error("Missing channel " + name);
                indices.push_back(it - a->names.begin());
                if (end == std::string::npos)
                    break;
                start = end + 1;
            }
        }
        auto b = make(Signal::Channels, a->width, indices.size());
        for (size_t i = 0; i < indices.size(); ++i) {
            int idx = indices[i];
            std::copy_n(a->values.begin() + idx * a->width, a->width,
                        b->values.begin() + i * a->width);
            b->names.push_back(size_t(idx) < a->names.size() ? a->names[idx] : std::to_string(idx));
        }
        b->format = a->format;
        b->values.pack(b->format);
        return Results{b};
    }
    if (k == "MergeChannels") {
        auto d = get(1);
        if (a->width != d->width)
            throw std::runtime_error("Sample counts differ");
        auto b = make(Signal::Channels, a->width, a->height + d->height);
        b->values = a->values;
        b->values.insert(b->values.end(), d->values.begin(), d->values.end());
        b->names = a->names;
        for (auto name : d->names) {
            auto original = name;
            int suffix = 2;
            while (std::find(b->names.begin(), b->names.end(), name) != b->names.end())
                name = original + "." + std::to_string(suffix++);
            b->names.push_back(name);
        }
        return Results{b};
    }
    if (k == "ReduceChannels") {
        bool across = choice(0) == 1;
        auto b = make(Signal::Channels, across ? a->width : 1, across ? 1 : a->height);
        for (int i = 0; i < int(b->values.size()); ++i) {
            int count = across ? a->height : a->width;
            float v = choice(1) == 1 ? INFINITY : choice(1) == 2 ? -INFINITY : 0;
            for (int j = 0; j < count; ++j) {
                float x = a->values[across ? j * a->width + i : i * a->width + j];
                v = choice(1) == 1 ? std::min(v, x) : choice(1) == 2 ? std::max(v, x) : v + x;
            }
            b->values[i] = choice(1) == 0 ? v / count : v;
        }
        b->names = across ? std::vector<std::string>{std::vector<std::string>{"mean", "min",
                                                                              "max"}[choice(1)]}
                          : a->names;
        return Results{b};
    }
    if (k == "MultiplyChannels") {
        auto d = get(1);
        if (a->height != d->height || (d->width != 1 && a->width != d->width))
            throw std::runtime_error("Invalid weights");
        auto b = copy(a);
        for (int y = 0; y < a->height; ++y)
            for (int x = 0; x < a->width; ++x)
                b->values[y * a->width + x] *= d->values[y * d->width + x % d->width];
        return Results{b};
    }

    return {};
}
} // namespace touch::ops
