# godot-byprod

A GDExtension binding for the [byProd](https://byprod.io/) interactive audio
runtime by Madrigal Ltd., for Godot 4.

This is a community binding. It is not affiliated with or endorsed by Madrigal Ltd.

## What it gives you

Five classes, usable from GDScript with no C# and no engine build:

| Class | What it is |
|---|---|
| `ByProdSoundManager` | the runtime: loads a compiled project, owns the listener and the mix |
| `ByProdEventDescription` | an authored event; makes instances, resolves parameter indices |
| `ByProdEventInstance` | one playing event: start/stop, parameters, signals, 3D attributes |
| `ByProdGroupBus` | a mix group (music, UI, SFX) and its volume |
| `ByProdStreamPump` | optional: routes a host-mixed manager through Godot's audio buses |

The job scheduler, bulk audio and the debug statistics of the C API are not bound
yet. They are additive — see `BYPROD_API_FUNCTIONS` in
[`src/bp_api.h`](src/bp_api.h), where adding a function is one line.

## Status

Verified against the byProd 0.5.2 beta runtime on Windows and Linux (x86_64):
the SDK's own sample project loads, its bank is fetched through the host, and
`event:/Drums` plays with its parameters driven from GDScript. macOS is neither
built nor tested.

## Requirements

- Godot 4.5 or newer (built against godot-cpp `4.5`; GDExtension keeps this
  loadable in later 4.x)
- the byProd SDK — tested against **0.5.2 beta**
- SCons and a C++17 toolchain

## Building

```bash
git clone --recurse-submodules <this repo>
cd godot-byprod
scons target=template_debug platform=windows   # or platform=linux
```

The build needs **no** byProd files at all: the wrapper opens the runtime by name
and resolves its symbols at load time, so nothing proprietary is linked, vendored
or downloaded during a build. Get the SDK yourself from
[byprod.io](https://byprod.io/#download) and drop its shared library into
`addons/byprod/bin/` next to the built extension:

- Windows — `byProd.dll`
- Linux — `libbyProd.so` (the casing matters)

If it is missing, the extension still loads and every call is a safe no-op;
`ByProdSoundManager.is_runtime_available()` returns `false` and
`get_last_error()` says what was tried.

### Known quirk, not this binding's

A headless *editor* session (`godot --headless --import`) finishes its work and
then exits with a segfault on Godot 4.7.2. This reproduces with godot-cpp's own
`test/` extension and with any extension that registers classes, so it is not
something this binding causes or can fix. Imports still complete, and running a
game (`godot --headless demo/demo.tscn`) exits cleanly.

## Using it

```gdscript
var manager := ByProdSoundManager.create()
if manager == null:
    push_error(ByProdSoundManager.get_last_error())
    return

# Wave data lives in .bybank files, which the runtime asks for by name rather
# than opening itself. Point it at the folder holding them before loading, or
# the project loads with nothing audible in it.
manager.set_bank_directory("res://audio")
manager.load_project(FileAccess.get_file_as_bytes("res://audio/project.byprod"))

var description := manager.get_event_description("event:/footstep")
var instance := description.create_instance()
instance.set_parameter("surface", 1.0)
instance.set_3d_attributes(global_position, Vector3.ZERO)
instance.start()
instance.release_when_finished()
```

Banks are read through Godot's `FileAccess`, so `res://` paths keep working inside
an exported PCK, where the runtime's own file I/O could not reach them. Use
`preload_bank("main")` after loading a project to fetch one up front instead of
waiting for the first event that needs it.

byProd's own diagnostics are forwarded to Godot's console (`[byProd] …`), and the
binding warns when the runtime's version is not the one it was written against.

Call `manager.update()` once per frame, and feed the listener from your camera:

```gdscript
func _process(_delta: float) -> void:
    var t := camera.global_transform
    manager.set_listener_transform(t.origin, -t.basis.z, t.basis.y)
    manager.update()
```

The manager is created right-handed, matching Godot's 3D space — byProd itself
defaults to a left-handed one, which would mirror every panned sound.

### Going through Godot's audio buses

By default byProd opens its own audio device and mixes on its own thread, and
Godot never sees a sample. To put its output under Godot's buses, effects and
volume settings instead, create the manager host-mixed and add a
`ByProdStreamPump` pointing at an `AudioStreamPlayer` whose stream is an
`AudioStreamGenerator`:

```gdscript
var manager := ByProdSoundManager.create_host_mixed(int(generator.mix_rate))
pump.sound_manager = manager
pump.player_path = player.get_path()
```

The pump then calls `update()` for you.

## Licence

The binding is MIT, see [LICENSE](LICENSE).

byProd itself is **not** covered by it: the runtime and editor are Madrigal Ltd.'s
own software under their beta terms, which are free for commercial and
non-commercial use and require crediting *"byProd audio engine, copyright
Madrigal Ltd."* in your game. Nothing of theirs is redistributed here.
