#version 330 core

in TES_OUT {
    vec2 texCoord;
} fs_in;

out vec4 FragColor;

uniform sampler2D textureSurface;
uniform bool wireframe;
uniform bool showControlPoints;

void main() {
    // 检查是否在边界上（边界定义为靠近0或1的区域）
    float edgeThreshold = 0.01;
    
    if (showControlPoints) {
        // 控制点显示为明亮的蓝色
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