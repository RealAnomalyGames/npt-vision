# NPT Vision

**NPT Vision** is a 2D animation editor developed by Neptune Corporation™ for its NPT Professional lineup.

The project aims to provide a lightweight desktop animation workflow with a traditional interface, frame-based timeline, layers, drawing tools, and image import support.

## Current Development Status

Version: **0.1.0 — Initial Development**

NPT Vision is currently in its foundational development stage. The initial interface is being established before full drawing and animation functionality is implemented.

### Planned Features

- Frame-by-frame 2D animation
- Drawing and erasing tools
- Animation timeline and layers
- Playback speed controls
- Onion skinning
- PNG, BMP, JPEG, and SVG import
- Native `.nvp` project files
- PNG sequence export
- Undo and redo
- Automated Windows builds through GitHub Actions

## Platform Support

The initial build targets 64-bit Windows 10 and Windows 11.

Windows 7, Vista, XP, 98, ME, and 95 are long-term compatibility goals requiring additional investigation and potentially separate builds. Compatibility with these operating systems is not yet guaranteed.

## Technology

- C++17
- Qt Widgets
- Qt SVG
- CMake
- GitHub Actions

## Building from Source

Install a compatible Visual Studio C++ toolchain, CMake 3.21 or newer, and Qt 6.8.3 with the MSVC 2022 64-bit kit and SVG module.

From the repository root, run:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"
cmake --build build --config Release
```

Adjust the Qt path if it is installed elsewhere.

The executable will be generated at `build/Release/NPTVision.exe`.

## Contributing

Bug reports, feature suggestions, and contributions are welcome through GitHub Issues and Pull Requests.

## Developer

Neptune Corporation

## License

The project license has not yet been selected. Check this repository for licensing information before redistributing the software.
