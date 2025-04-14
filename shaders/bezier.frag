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
    bool isOnEdge = fs_in.texCoord.x < edgeThreshold || 
                    fs_in.texCoord.x > 1.0 - edgeThreshold ||
                    fs_in.texCoord.y < edgeThreshold || 
                    fs_in.texCoord.y > 1.0 - edgeThreshold;
    
    if (showControlPoints) {
        // 控制点显示为明亮的蓝色
        FragColor = vec4(0.0, 0.5, 1.0, 1.0);
    }
    else if (wireframe) {
        if (isOnEdge) {
            // 边界线显示为绿色
            FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        } else {
            // 线框显示为红色
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    } 
    else {
        // 正常显示纹理，但边界有特殊颜色
        if (isOnEdge) {
            // 边界区域混合绿色
            vec4 texColor = texture(textureSurface, fs_in.texCoord);
            FragColor = mix(texColor, vec4(0.0, 1.0, 0.0, 1.0), 0.3);
        } else {
            // 正常显示纹理
            FragColor = texture(textureSurface, fs_in.texCoord);
        }
    }
} 