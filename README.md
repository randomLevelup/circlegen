# Circlegen - Minimalistic Circle-Based Image Generation

Circlegen is an image processing algorithm that creates artistic, minimalistic representations of images using partially filled-in circles. To generate and color the circles, I use a combination of point sampling, [RookFighter's gradient descent](https://github.com/Rookfighter/gradient-descent-cpp), and a cool new hashing trick.

This project was inspired by [this](https://x.com/TerribleMaps/status/1867903117548769654) X post. I'm not sure where the original image came from. If you do, pls let me know.

Circlegen features WebAssembly support, and I'm hosting a [live demo](https://www.youtube.com/watch?v=dQw4w9WgXcQ) on my website.

## Examples
| ![ex3](/examples/outputs/world.png) |
|-|

| ![ex1](/examples/outputs/cartman.png) | ![ex2](/examples/outputs/ironman.png) |
|-|-|
Check out the [examples](/examples/outputs) folder for more

## Build Instructions

### Prerequisites

- C++ compiler with C++11 support
- Eigen3 library (for gradient descent)
- libpng & libjpeg (for native builds)
- Emscripten (for WebAssembly builds)

### Building Natively
To build an executable on your system, clone the repo and run the following:
```bash
cd circlegen
mkdir build && cd build
cmake .. && make
```

### Building for WebAssembly:
To build the WASM object, first activate your emscripten environment (run `emsdk activate latest`)

Then clone the repo and run the following:
```bash
cd circlegen/wasm/src
./build.sh
```

## Usage

### Native Build -- Command Line Interface

```bash
circlegen path/to.input.jpg
```
I'm working on adding more arguments for better image customization. For now, if you want to change the number of circles, update the `generateCircles` call at line 40 of [main.cpp](/main.cpp#L40) and rebuild.

### Web Interface

Reference [/wasm/index.html](/wasm/index.html) for an example of how to load the compiled `circlegen.wasm` module.

## How It Works

1. **Point Sampling**:
   - The input image is first resampled with jitter for better edge detection
   - A Sobel filter is applied to detect edges in the image
   - Points are then sampled along the detected edges based on some threshold

3. **Circle Generation**:
   - Random pairs of points are selected to initialize circles
   - The [Barzilai-Borwein Method](https://en.wikipedia.org/wiki/Barzilai-Borwein_method) is used to refine circle parameters to best fit the points
   - After generating a new circle, points that lie close to it are removed to prevent overlapping circles

4. **Color Quantization**:
   - The image is divided into regions based on circle intersections
   - Each region's colors are quantized to a representative median color
   - The combination of circles and quantized colors creates the final artistic effect
