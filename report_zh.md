# 贝塞尔曲面渲染器实现报告

## 1. 贝塞尔曲面的数学原理

### 1.1 贝塞尔曲线基础

贝塞尔曲线是一种参数化曲线，由法国工程师皮埃尔·贝塞尔（Pierre Bézier）于1962年首次提出，最初用于汽车设计。贝塞尔曲线由一组控制点定义，其最基本的数学表达式为：

$$B(t) = \sum_{i=0}^{n} P_i B_{i,n}(t), \quad t \in [0,1]$$

其中：
- $P_i$ 是控制点
- $B_{i,n}(t)$ 是伯恩斯坦多项式
- $n$ 是曲线的阶数（控制点数量减1）

伯恩斯坦多项式定义为：

$$B_{i,n}(t) = \binom{n}{i} t^i (1-t)^{n-i}$$

其中 $\binom{n}{i}$ 是二项式系数，表示从n个元素中选择i个的方式数量。

### 1.2 贝塞尔曲面扩展

贝塞尔曲面是贝塞尔曲线在二维参数空间的推广，由两个参数u和v控制。对于由（m+1）×（n+1）个控制点定义的贝塞尔曲面，其数学表达式为：

$$S(u,v) = \sum_{i=0}^{m} \sum_{j=0}^{n} P_{ij} B_{i,m}(u) B_{j,n}(v), \quad u,v \in [0,1]$$

其中：
- $P_{ij}$ 是控制点网格
- $B_{i,m}(u)$ 和 $B_{j,n}(v)$ 是u方向和v方向的伯恩斯坦多项式

在我们的项目中，使用了5×5的控制点网格，因此m=n=4，实现了一个4阶贝塞尔曲面。

### 1.3 计算实现

在着色器中，我们通过以下步骤计算贝塞尔曲面上的点：

1. 计算伯恩斯坦多项式
2. 对每个控制点应用相应的多项式权重
3. 累加得到最终位置

这在曲面细分评估着色器(TESE)中实现，代码片段如下：

```glsl
// 伯恩斯坦多项式计算
float bernstein(int i, int n, float t) {
    // 计算二项式系数 C(n,i)
    float coeff = 1.0;
    for (int j = 1; j <= i; j++) {
        coeff *= float(n - j + 1) / float(j);
    }
    
    return coeff * pow(t, float(i)) * pow(1.0 - t, float(n - i));
}

// 计算贝塞尔曲面点
vec3 bezierSurface(float u, float v) {
    vec3 result = vec3(0.0);
    
    // 5x5 控制点的贝塞尔曲面
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

## 2. OpenGL渲染管线

现代OpenGL渲染管线包含多个阶段，其中本项目重点利用了曲面细分阶段来实现贝塞尔曲面。

### 2.1 顶点着色器 (Vertex Shader)

顶点着色器是渲染管线的第一阶段。在我们的项目中，顶点着色器非常简单，只负责接收控制点数据并传递给后续阶段：

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

### 2.2 曲面细分控制着色器 (Tessellation Control Shader, TCS)

TCS负责设置曲面细分级别和准备控制点数据。在我们的项目中，TCS接收用户设定的细分级别，并将其应用到曲面的内部和外部边界：

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
    // 传递顶点属性
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].texCoord = tcs_in[gl_InvocationID].texCoord;
    
    // 保存原始控制点位置
    tcs_out[gl_InvocationID].position = gl_in[gl_InvocationID].gl_Position.xyz;
    
    // 只在第一个控制点设置细分级别
    if (gl_InvocationID == 0) {
        // 设置外部边界和内部细分级别
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;
        
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}
```

### 2.3 曲面细分评估着色器 (Tessellation Evaluation Shader, TES)

TES是整个贝塞尔曲面实现的核心。它接收细分后生成的参数坐标(u,v)，并根据贝塞尔曲面方程计算对应的曲面点坐标：

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

// 伯恩斯坦多项式和贝塞尔曲面计算函数...

void main() {
    // 获取参数 u 和 v
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    
    // 计算贝塞尔曲面上的点
    vec3 pos = bezierSurface(u, v);

    // 应用变换矩阵
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    // 计算并传递纹理坐标
    tes_out.texCoord = vec2(clamp(u, 0.0, 1.0), clamp(v, 0.0, 1.0));
}
```

### 2.4 片段着色器 (Fragment Shader)

片段着色器负责最终的颜色计算，包括纹理应用和线框模式的处理：

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
        // 控制点显示为蓝色
        FragColor = vec4(0.0, 0.5, 1.0, 1.0);
    }
    else if (wireframe) {
        // 线框显示为红色
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } 
    else {
        // 正常显示纹理
        FragColor = texture(textureSurface, fs_in.texCoord);
    }
}
```

## 3. 不同渲染模式

这个项目支持两种主要的渲染模式：实体纹理模式和线框模式。

### 3.1 实体纹理模式

在实体纹理模式下：
- 通过片段着色器对贝塞尔曲面应用纹理
- 使用`texture`函数采样纹理并将纹理颜色应用到曲面
- 通过`glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)`设置OpenGL绘制模式为填充模式

```cpp
// 设置为填充模式
glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

// 在片段着色器中
FragColor = texture(textureSurface, fs_in.texCoord);
```

### 3.2 线框模式

在线框模式下：
- 只渲染多边形的边缘，显示网格结构
- 通过`glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`设置OpenGL绘制模式为线框模式
- 使用红色绘制线框，使其更加明显
- 额外显示蓝色控制点，以直观展示控制点对曲面的影响

```cpp
// 设置为线框模式
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

// 在片段着色器中
FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // 红色线框
```

## 4. 曲面细分着色器的关键实现

### 4.1 TCS (Tessellation Control Shader)

TCS的主要功能是设置细分级别，决定曲面的详细程度。在我们的项目中，细分级别可通过键盘上下键动态调整，范围从1到50。细分级别越高，生成的曲面越平滑，但也需要更多的计算资源。

TCS的关键特性：
- 声明`layout (vertices = 25) out`，表示输出25个控制点（5×5网格）
- 将控制点数据从顶点着色器传递到TES
- 设置内部和外部细分级别（通过`gl_TessLevelOuter`和`gl_TessLevelInner`）
- 细分级别由用户通过键盘控制，存储在uniform变量`tessLevel`中

### 4.2 TES (Tessellation Evaluation Shader)

TES是实现贝塞尔曲面数学计算的核心。在细分阶段生成的每个新顶点上，TES计算其在曲面上的实际位置。

TES的关键特性：
- 声明`layout(quads, equal_spacing, ccw) in`，表示使用四边形拓扑、均匀细分和逆时针顶点顺序
- 实现伯恩斯坦多项式计算函数
- 实现贝塞尔曲面插值函数
- 通过参数(u,v)计算曲面上点的3D坐标
- 应用变换矩阵（模型、视图、投影）得到最终屏幕坐标
- 计算并输出对应的纹理坐标

## 5. 总结

本项目通过OpenGL的曲面细分着色器成功实现了交互式贝塞尔曲面渲染器。主要特点包括：

1. 使用5×5控制点网格定义曲面
2. 通过TCS动态控制细分级别
3. 在TES中实现贝塞尔曲面数学计算
4. 支持纹理映射和线框显示模式
5. 提供交互式控制，包括相机操作和细分级别调整

这个实现展示了现代OpenGL着色器阶段的强大功能，特别是曲面细分着色器在计算几何建模中的应用。通过程序化生成复杂曲面，我们可以用少量控制点实现高度复杂和可交互的3D模型。 