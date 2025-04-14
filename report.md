# Bezier Surface Renderer Implementation Report

## 1. Mathematical Principles of Bezier Surfaces

### 1.1 Bezier Curve Fundamentals

A Bezier curve is a parametric curve first introduced by French engineer Pierre Bézier in 1962, originally developed for automotive design. Bezier curves are defined by a set of control points, with their most basic mathematical expression being:

$$B(t) = \sum_{i=0}^{n} P_i B_{i,n}(t), \quad t \in [0,1]$$

Where:
- $P_i$ represents the control points
- $B_{i,n}(t)$ are Bernstein polynomials
- $n$ is the order of the curve (number of control points minus 1)

The Bernstein polynomial is defined as:

$$B_{i,n}(t) = \binom{n}{i} t^i (1-t)^{n-i}$$

Where $\binom{n}{i}$ is the binomial coefficient, representing the number of ways to choose i elements from a set of n elements.

### 1.2 Extension to Bezier Surfaces

A Bezier surface is an extension of Bezier curves into a two-dimensional parameter space, controlled by two parameters u and v. For a Bezier surface defined by (m+1)×(n+1) control points, the mathematical expression is:

$$S(u,v) = \sum_{i=0}^{m} \sum_{j=0}^{n} P_{ij} B_{i,m}(u) B_{j,n}(v), \quad u,v \in [0,1]$$

Where:
- $P_{ij}$ is the grid of control points
- $B_{i,m}(u)$ and $B_{j,n}(v)$ are Bernstein polynomials in the u and v directions

In our project, we use a 5×5 control point grid, so m=n=4, implementing a 4th-order Bezier surface.

### 1.3 Computational Implementation

In our shader implementation, we calculate points on the Bezier surface through the following steps:

1. Compute the Bernstein polynomials
2. Apply the corresponding polynomial weights to each control point
3. Accumulate to obtain the final position

This is implemented in the Tessellation Evaluation Shader (TESE), as shown in the following code snippet:

```glsl
// Bernstein polynomial calculation
float bernstein(int i, int n, float t) {
    // Calculate binomial coefficient C(n,i)
    float coeff = 1.0;
    for (int j = 1; j <= i; j++) {
        coeff *= float(n - j + 1) / float(j);
    }
    
    return coeff * pow(t, float(i)) * pow(1.0 - t, float(n - i));
}

// Calculate Bezier surface point
vec3 bezierSurface(float u, float v) {
    vec3 result = vec3(0.0);
    
    // 5x5 control points Bezier surface
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            int idx = i * 5 + j;
            float basis_i = bernstein(i, 4, v); 
            float basis_j = bernstein(j, 4, u);
            float coeff = basis_i * basis_j;
            result += coeff * tes_in[idx].position;
        }
    }
    
    return result;
}
```

## 2. OpenGL Rendering Pipeline

The modern OpenGL rendering pipeline consists of multiple stages, with this project focusing specifically on the tessellation stage to implement the Bezier surface.

### 2.1 Vertex Shader

The vertex shader is the first stage in the rendering pipeline. In our project, the vertex shader is quite simple, responsible only for receiving control point data and passing it to subsequent stages:

```glsl
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out VS_OUT {
    vec2 texCoord;
} vs_out;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vs_out.texCoord = aTexCoord;
}
```

### 2.2 Tessellation Control Shader (TCS)

The TCS is responsible for setting the tessellation level and preparing the control point data. In our project, the TCS receives the user-defined tessellation level and applies it to both the internal and external boundaries of the surface:

```glsl
#version 330 core

layout (vertices = 25) out;

in VS_OUT {
    vec2 texCoord;
} tcs_in[];

out TCS_OUT {
    vec2 texCoord;
    vec3 position;
} tcs_out[];

uniform float tessLevel;

void main() {
    // Pass vertex attributes
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].texCoord = tcs_in[gl_InvocationID].texCoord;
    
    // Store original control point position
    tcs_out[gl_InvocationID].position = gl_in[gl_InvocationID].gl_Position.xyz;
    
    // Set tessellation levels only in the first control point
    if (gl_InvocationID == 0) {
        // Set external and internal tessellation levels
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
        
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}
```

### 2.3 Tessellation Evaluation Shader (TES)

The TES is the core of the entire Bezier surface implementation. It receives the parameter coordinates (u,v) generated after tessellation and calculates the corresponding surface point coordinates based on the Bezier surface equation:

```glsl
#version 330 core

layout(quads, equal_spacing, ccw) in;

in TCS_OUT {
    vec2 texCoord;
    vec3 position;
} tes_in[];

out TES_OUT {
    vec2 texCoord;
} tes_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Bernstein polynomial and Bezier surface calculation functions...

void main() {
    // Get parameters u and v
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    
    // Calculate point on the Bezier surface
    vec3 pos = bezierSurface(u, v);

    // Apply transformation matrices
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    // Calculate and pass texture coordinates
    tes_out.texCoord = vec2(clamp(u, 0.0, 1.0), clamp(v, 0.0, 1.0));
}
```

### 2.4 Fragment Shader

The fragment shader is responsible for final color calculations, including texture application and wireframe mode processing:

```glsl
#version 330 core

in TES_OUT {
    vec2 texCoord;
} fs_in;

out vec4 FragColor;

uniform sampler2D textureSurface;
uniform bool wireframe;
uniform bool showControlPoints;

void main() {
    if (showControlPoints) {
        // Show control points as blue
        FragColor = vec4(0.0, 0.5, 1.0, 1.0);
    }
    else if (wireframe) {
        // Show wireframe as red
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } 
    else {
        // Normal texture display
        FragColor = texture(textureSurface, fs_in.texCoord);
    }
}
```

## 3. Different Rendering Modes

This project supports two main rendering modes: solid texture mode and wireframe mode.

### 3.1 Solid Texture Mode

In solid texture mode:
- The texture is applied to the Bezier surface via the fragment shader
- The `texture` function is used to sample the texture and apply the texture color to the surface
- OpenGL's drawing mode is set to fill mode using `glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)`

```cpp
// Set to fill mode
glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

// In the fragment shader
FragColor = texture(textureSurface, fs_in.texCoord);
```

### 3.2 Wireframe Mode

In wireframe mode:
- Only the edges of polygons are rendered, showing the mesh structure
- OpenGL's drawing mode is set to line mode using `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`
- Red color is used for the wireframe to make it more visible
- Blue control points are additionally displayed to visually demonstrate the influence of control points on the surface

```cpp
// Set to wireframe mode
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

// In the fragment shader
FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Red wireframe
```

## 4. Key Implementation of Tessellation Shaders

### 4.1 TCS (Tessellation Control Shader)

The main function of the TCS is to set the tessellation level, determining the level of detail of the surface. In our project, the tessellation level can be dynamically adjusted using keyboard up/down keys, ranging from 1 to 50. Higher tessellation levels result in smoother surfaces but require more computational resources.

Key features of the TCS:
- Declares `layout (vertices = 25) out`, indicating output of 25 control points (5×5 grid)
- Passes control point data from the vertex shader to the TES
- Sets internal and external tessellation levels (via `gl_TessLevelOuter` and `gl_TessLevelInner`)
- Tessellation level is controlled by the user via keyboard and stored in the uniform variable `tessLevel`

### 4.2 TES (Tessellation Evaluation Shader)

The TES is the core of the Bezier surface mathematical calculation. For each new vertex generated in the tessellation stage, the TES calculates its actual position on the surface.

Key features of the TES:
- Declares `layout(quads, equal_spacing, ccw) in`, indicating the use of quad topology, uniform subdivision, and counterclockwise vertex order
- Implements the Bernstein polynomial calculation function
- Implements the Bezier surface interpolation function
- Calculates 3D coordinates on the surface using parameters (u,v)
- Applies transformation matrices (model, view, projection) to get the final screen coordinates
- Calculates and outputs the corresponding texture coordinates

## 5. Conclusion

This project successfully implements an interactive Bezier surface renderer using OpenGL's tessellation shaders. The main features include:

1. Using a 5×5 control point grid to define the surface
2. Dynamically controlling tessellation level via TCS
3. Implementing Bezier surface mathematical calculations in TES
4. Supporting texture mapping and wireframe display modes
5. Providing interactive controls, including camera operations and tessellation level adjustment

This implementation demonstrates the powerful capabilities of modern OpenGL shader stages, especially the application of tessellation shaders in computational geometric modeling. By programmatically generating complex surfaces, we can achieve highly complex and interactive 3D models with a small number of control points. 