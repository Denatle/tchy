# Node plugin SDK v1

Include `touch/plugin.h`. Build a shared library with the target architecture of the host and export `touch_plugin_get_api(uint32_t)`. The SDK has no dependency on the C++ standard library, SDL or ImGui. `plugins/example.cpp` builds as a separate module and demonstrates a Control source and Video effect.

The entry point returns a static `TouchPluginApi`. Return null for an unsupported host ABI version. The host checks ABI and structure sizes, callbacks, descriptors and duplicate identifiers before publishing the complete set of operators.

Each `TouchOperatorApi` contains a JSON descriptor and create/destroy/process callbacks. Descriptors use globally unique stable IDs:

```json
{
  "id": "vendor.effect",
  "label": "My Effect",
  "inputs": [{"id": "image", "signal": 1}],
  "outputs": [{"id": "result", "signal": 1}],
  "params": [{"name": "Amount", "default": 0.5, "min": 0, "max": 1}],
  "choices": [{"name": "Mode", "items": ["A", "B"]}],
  "video_rate": 1,
  "cacheable": 1
}
```

Signal IDs are Video=1, Audio=2, Control=3, Table=4, Mask=5, Channels=6. Integer port declarations are accepted as shorthand and assigned positional keys. Explicit IDs are recommended for external plugins. Parameter port keys are `param.<name>`; parameter names therefore form part of the saved compatibility contract.

Create returns an opaque non-null instance. Process receives borrowed immutable input buffers, resolved parameter values, choices, time and node JSON with file, table and channel selection settings. Allocate outputs through `TouchHost.allocate_output`. The host owns these buffers; pointers may be used only during the current process call. A new output allocation for a port replaces its previous allocation. Do not retain pointers to input or output data. Return zero on success; on failure return nonzero and write a bounded, NUL-terminated error message. Exceptions must never cross the C boundary.

All ABI sample arrays are f32. `storage_format` requests host storage as f32=0, f16=1 or u8 UNORM=2 after process returns. Video is interleaved RGBA, Mask has one component, Channels uses one row per named channel, and Audio is 480 interleaved frames at 48 kHz. `width` is samples per row, `height` is row count, `components` is interleaved component count. Table rows and channel names travel as JSON metadata through `set_metadata` and `TouchValue.metadata_json`:

```json
{"names": ["left", "right"], "rows": [["value", "0.5"]]}
```

Callbacks run serially on the graph worker; UI code is not exposed. Plugins may manage their own worker threads, but destroy must join them and release all resources. A native plugin can crash its host; v1 provides no process isolation.

Place libraries in `plugins` beside the executable, in the per-user Touch preferences directory's `plugins` subdirectory, or choose Load Plugin. Replacement requires Stop and unload plugins, replacing the file, and loading again. ABI compatibility does not make a single binary portable between operating systems or CPU architectures.
