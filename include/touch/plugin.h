#ifndef TOUCH_PLUGIN_H
#define TOUCH_PLUGIN_H
#include <stddef.h>
#include <stdint.h>
#ifdef _WIN32
#define TOUCH_EXPORT __declspec(dllexport)
#else
#define TOUCH_EXPORT __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define TOUCH_ABI_VERSION 1u
/* Data is borrowed for one process call. Outputs are allocated and owned by
 * host. Audio is interleaved; channels are row-major; images are interleaved
 * RGBA. All ABI samples are f32; storage_format describes requested host
 * quantization. */
typedef struct TouchValue {
    uint32_t signal, width, height, components, storage_format;
    const float* samples;
    size_t sample_count;
    const char* metadata_json;
} TouchValue;
typedef struct TouchHost {
    uint32_t abi_version, struct_size;
    void* context;
    float* (*allocate_output)(void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    int (*set_metadata)(void*, uint32_t, const char*);
} TouchHost;
typedef struct TouchProcess {
    uint32_t struct_size, width, height;
    uint64_t frame;
    double seconds;
    const float* parameters;
    size_t parameter_count;
    const int32_t* choices;
    size_t choice_count;
    const TouchValue* inputs;
    size_t input_count;
    const char* node_json;
} TouchProcess;
typedef struct TouchOperatorApi {
    uint32_t struct_size;
    const char* descriptor_json;
    void* (*create)(void);
    void (*destroy)(void*);
    int (*process)(void*, const TouchHost*, const TouchProcess*, char*, size_t);
} TouchOperatorApi;
typedef struct TouchPluginApi {
    uint32_t abi_version, struct_size;
    size_t operator_count;
    const TouchOperatorApi* operators;
} TouchPluginApi;
typedef const TouchPluginApi* (*TouchGetApi)(uint32_t);
TOUCH_EXPORT const TouchPluginApi* touch_plugin_get_api(uint32_t host_abi);
#ifdef __cplusplus
}
#endif
#endif
