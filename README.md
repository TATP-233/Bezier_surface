# Bezier Surface Renderer

This is a Bezier surface renderer implemented using OpenGL, which achieves efficient Bezier surface generation and rendering through tessellation shaders.

[中文文档](README_zh.md)

[report](report.md)

## Features

- Uses 5x5 (25 total) control points to generate a Bezier surface
- Sets subdivision level via Tessellation Control Shader (TCS)
- Calculates new vertex coordinates and texture coordinates based on the mathematical equation of Bezier surface using Tessellation Evaluation Shader (TES)
- Supports adjusting surface smoothness via keyboard
- Supports wireframe mode display
- Applies texture to the Bezier surface

## System Requirements

- OpenGL 3.3+
- GLFW3
- GLEW
- C++17 compatible compiler
- CMake 3.10+

## Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/bezier_surface.git
cd bezier_surface

# Create build directory
mkdir build && cd build

# Configure and build the project
cmake ..
make
```

## Running the Application

```bash
./bezier_surface
```

## Controls

- **Up Arrow**: Increase tessellation level (increase smoothness)
- **Down Arrow**: Decrease tessellation level (decrease smoothness)
- **W key**: Toggle wireframe mode
- **ESC key**: Exit program

### Camera Controls:
- **Left Mouse Button Drag**: Rotate view
- **Right Mouse Button Drag**: Pan view
- **Mouse Wheel**: Zoom

## Project Structure

```
bezier_surface/
├── include/            # Header files
│   ├── shader.h        # Shader class definition
│   └── stb_image.h     # Image loading library
├── shaders/            # Shader files
│   ├── bezier.vert     # Vertex shader
│   ├── bezier.tesc     # Tessellation control shader
│   ├── bezier.tese     # Tessellation evaluation shader
│   └── bezier.frag     # Fragment shader
├── src/                # Source files
│   ├── main.cpp        # Main program
│   └── shader.cpp      # Shader class implementation
├── textures/           # Texture resources
│   └── surface.jpg     # Surface texture
└── CMakeLists.txt      # CMake build configuration
``` 