# IMAGR

An image viewer that works using dynamic libraries.
Current plugins are: decoders for stb_image (png, jpeg, and bmp), qoi, and gif.

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

## Dependencies

- SDL3
- SDL3_ttf
- inicpp

plugins:
- stb: stb_image
- gif: giflib
- qoi: qoi