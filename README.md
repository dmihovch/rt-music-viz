# rt-music-viz

Real-time music visualizer written in C17 for Linux.

It captures the default audio sink's monitor stream through the native PipeWire
API, buffers the samples, computes a magnitude spectrum, and renders it with
raylib.

## Dependencies

Build tools:

- `cc` (GCC) and `make`
- `pkg-config`
- `bear` — generates `compile_commands.json` for the LSP (installed)

System packages:

- `libpipewire-0.3-dev` — PipeWire headers and pkg-config file (installed)
- raylib — installed at `/usr/local` (header and static library)
- OpenGL and X11 development files

No external FFT library is used; the analyzer hand-rolls its own transform.

## Build

```sh
make
```

The Makefile compiles the sources directly; no other build system is involved.
The binary is written to `build/rt_music_viz`.

Other targets:

- `make run` — build and run the binary
- `make valgrind` — build and run the binary under valgrind
- `make clean` — remove the build directory
- `make compile_commands` — regenerate `compile_commands.json` for the LSP

## Editor / LSP

Run `make compile_commands` to (re)generate `compile_commands.json` at the repo
root with `bear`. `.clangd` points clangd at it, so the language server resolves
the PipeWire and raylib includes and the exact compiler flags without extra
configuration.

## Layout

- `src/main.c` — entry point; wires the modules together
- `src/audio_capture.c` — PipeWire monitor capture
- `src/audio_buffer.c` — single-producer/single-consumer sample ring buffer
- `src/analyzer.c` — windowing and FFT magnitude spectrum
- `src/visualizer.c` — raylib rendering
- `include/` — public headers for the modules above

The module bodies are stubs; each function carries a `TODO` describing the
intended implementation.
