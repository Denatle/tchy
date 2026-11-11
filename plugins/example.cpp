#include <cstdio>
#include <touch/plugin.h>
namespace {
void* create() {
    return new int(0);
}
void destroy(void* p) {
    delete static_cast<int*>(p);
}
int constant(void*, const TouchHost* h, const TouchProcess* p, char*, size_t) {
    auto out = h->allocate_output(h->context, 0, 3, 1, 1, 1, 0);
    if (!out)
        return -1;
    out[0] = p->parameters[0];
    return 0;
}
int invert(void*, const TouchHost* h, const TouchProcess* p, char* error, size_t size) {
    if (!p->input_count || !p->inputs[0].samples) {
        std::snprintf(error, size, "Connect an image");
        return -1;
    }
    auto a = p->inputs[0];
    auto out = h->allocate_output(h->context, 0, 1, a.width, a.height, 4, 0);
    if (!out)
        return -1;
    for (size_t i = 0; i < a.sample_count; ++i)
        out[i] = i % 4 == 3 ? a.samples[i] : 1 - a.samples[i];
    return 0;
}
const TouchOperatorApi operators[] = {
    {sizeof(TouchOperatorApi),
     R"({"id":"example.constant","label":"Plugin Constant","inputs":[],"outputs":[3],"params":[{"name":"Value","default":0.5,"min":0,"max":1}]})",
     create, destroy, constant},
    {sizeof(TouchOperatorApi),
     R"({"id":"example.invert","label":"Plugin Invert","inputs":[1],"outputs":[1],"params":[]})",
     create, destroy, invert}};
const TouchPluginApi api = {TOUCH_ABI_VERSION, sizeof(TouchPluginApi), 2, operators};
} // namespace
extern "C" TOUCH_EXPORT const TouchPluginApi* touch_plugin_get_api(uint32_t version) {
    return version == TOUCH_ABI_VERSION ? &api : nullptr;
}
