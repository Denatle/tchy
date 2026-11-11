# Architecture

`src/app/main.cpp` is the composition root. It creates media and audio adapters, the registry, plugin manager, project repository, clock, runtime and editor in lifetime order. The runtime is destroyed before its dependencies. There is no global service locator.

| Subsystem | Contract | Implementation |
| --- | --- | --- |
| graph | Nodes, stable port keys, edges, validation | Transactional connection/routing edits, iterative cycle detection |
| data | Immutable published signal values | Float, packed IEEE f16 and u8 UNORM samples, lazy float access for external APIs |
| operators | Descriptor, Operator, Registry | Catalog/module, factory, separate control, channels, masks, spatial, effects and audio families |
| media | MediaFactory, MediaSource | libav decoding, timelines, platform capture, SDL microphone |
| audio | AudioOutputs, AudioOutput, Clock | SDL audio output and steady clock |
| engine | Evaluator, Runtime, Monitor | Dependency ordering, per-node instances/cache, worker lifecycle |
| projects | ProjectStore | JSON codec and atomic file replacement |
| settings | PreferencesStore | Per-user JSON preferences |
| plugins | C SDK and PluginManager | SDL shared library loader and independent ABI adapter |
| editor | Document | Edit transactions, undo/redo, selection, duplicate/delete |
| ui | Editor dependencies | Canvas drawing/input, gestures, inspector, catalog, files and shortcuts |

Graph operations know nothing about SDL or ImGui. Operator implementations receive resolved parameters and input values; parameter modulation maps a Control value in 0..1 onto the parameter's declared range. A plugin descriptor uses the same registry and Operator interface as the built-ins. Adding a plugin never changes engine dispatch.

The UI sends graph snapshots under a mutex. A single worker evaluates reachable output branches, publishes immutable values and owns operator instances and audio output. Audio ticks use 480 frames at 48 kHz. Video operators retain their latest result between video deadlines. Pure operators reuse outputs when parameters and input identities match. Media source identity changes reopen its instance.

Stop waits until operator instances and audio outputs are released. Only then may the UI unregister plugins. Registry insertion is atomic for all operators in one library. Each plugin instance holds a shared library lifetime token; host buffers never require plugin code to free them.

File decoding currently runs in the graph worker. FFmpeg I/O has a two-second interrupt deadline; this is not a hard realtime audio engine. Expensive CPU graphs can miss their audio deadline. GPU processing and isolated plugin processes remain separate future work.

The family implementation files deliberately share small raster helpers and parameter types. They do not expose ImGui, platform handles or engine synchronization. Test fixtures inject media/audio implementations at those interfaces.
