# Algorithm Visualizer

A static web app that takes an image (JPEG or PNG), lets you configure a few parameters, and generates a animation visualizing different sorting algorithms or content-aware image resizing (seam carving) on your image.

The core is written in C++23 and compiled to WebAssembly (WASM) using Emscripten. It can also be built as a native command-line application.

## Features

- Sorting algorithms:
  - Bubble Sort
  - Comb Sort
  - Insertion Sort
  - Selection Sort
  - Merge Sort
  - Heap Sort
  - Quick Sort

- Seam Carving: Content-aware image resizing

- WebP Export: Generates an animated WebP file of the visualization

## Tech Stack & Dependencies

- C++23
- CMake (version 3.30+)
- [Emscripten](https://emscripten.org/)
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
- [libwebp](https://github.com/webmproject/libwebp)

## How to Build

### Option A: Native CLI Build

To compile the native terminal-based application:

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Option B: WebAssembly Build (WASM)

To compile the project for the web using Emscripten:

```bash
mkdir build_wasm && cd build_wasm
emcmake cmake ..
cmake --build . --config Release
```

This will generate `AlgoVisualizer_wasm.js` and the corresponding WASM binaries for the frontend.

---

## Showcase

### MergeSort

<img src="/showcase/merge_sort.webp" width='50%'>

### Seam Carving

<img src="/showcase/seam_carving.webp" width='50%'>
