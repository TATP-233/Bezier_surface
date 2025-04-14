# 贝塞尔曲面渲染器

这是一个使用OpenGL实现的贝塞尔曲面渲染器，通过曲面细分着色器实现高效的贝塞尔曲面生成和渲染。

[English Document](README.md)

[报告](report_zh.md)


## 功能特点

- 使用5x5（共25个）控制点生成贝塞尔曲面
- 通过曲面细分控制着色器（TCS）设置细分级别
- 通过曲面细分评估着色器（TES）根据贝塞尔曲面的数学方程计算新顶点坐标和纹理坐标
- 支持通过键盘调整曲面平滑度
- 支持线框模式显示
- 支持在曲面上应用纹理

## 系统要求

- OpenGL 3.3+
- GLFW3
- GLEW
- C++17兼容编译器
- CMake 3.10+

## 构建指南

```bash
# 克隆仓库
git clone https://github.com/yourusername/bezier_surface.git
cd bezier_surface

# 创建构建目录
mkdir build && cd build

# 配置和构建项目
cmake ..
make
```

## 运行方法

```bash
./bezier_surface
```

## 操作说明

- **上箭头**：增加曲面细分级别（增加平滑度）
- **下箭头**：减少曲面细分级别（降低平滑度）
- **W键**：切换线框模式
- **ESC键**：退出程序

### 相机控制：
- **鼠标左键拖动**：旋转视角
- **鼠标右键拖动**：平移视角
- **鼠标滚轮**：缩放

## 项目结构

```
bezier_surface/
├── include/            # 头文件
│   ├── shader.h        # 着色器类定义
│   └── stb_image.h     # 图像加载库
├── shaders/            # 着色器文件
│   ├── bezier.vert     # 顶点着色器
│   ├── bezier.tesc     # 曲面细分控制着色器
│   ├── bezier.tese     # 曲面细分评估着色器
│   └── bezier.frag     # 片段着色器
├── src/                # 源文件
│   ├── main.cpp        # 主程序
│   └── shader.cpp      # 着色器类实现
├── textures/           # 纹理资源
│   └── surface.jpg     # 曲面纹理
└── CMakeLists.txt      # CMake构建配置
``` 