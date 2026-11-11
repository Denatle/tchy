#include <touch/plugin.h>
extern "C" TOUCH_EXPORT const TouchPluginApi* touch_plugin_get_api(uint32_t) {
    static const TouchPluginApi api = {999, sizeof(TouchPluginApi), 0, nullptr};
    return &api;
}
