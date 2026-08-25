# IMAGR

An image viewer that works using dynamic libraries.
Current plugins are: stb_image, qoi, and static (first frame only) gif support.

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
