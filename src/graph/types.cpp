#include <algorithm>
#include <touch/graph/types.hpp>
namespace touch {
Value data(Signal t, int w, int h, int c, float fill) {
    if (w < 1 || h < 1 || c < 1 || uint64_t(w) * h * c > (t == Signal::Video ? 67108864 : 16777216))
        throw std::runtime_error("Data exceeds size limit");
    auto d = std::make_shared<Data>();
    d->type = t;
    d->width = w;
    d->height = h;
    d->components = c;
    d->values.assign(size_t(w) * h * c, fill);
    return d;
}
Node* Graph::find(Id id) {
    for (auto& n : nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}
const Node* Graph::find(Id id) const {
    for (auto& n : nodes)
        if (n.id == id)
            return &n;
    return nullptr;
}

} // namespace touch
