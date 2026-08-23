# IMAGR

An imager viewer that works using static libraries, at the moment I only have stb_image as a plugin.

This aims to be something akin to VST but for images.
It could possibly be used in an image editor too, although I won't promise anything.

it's very primative at the moment, as it only supports image decoding.

## Building

To build it with ninja:

```bash
cmake -B build -G Ninja
cd build
ninja
```

on linux you might need to fix SDL3 in the CMakeLists.txt, as I couldn't get it working...

once built, drag and drop an image onto the executable for it to work.