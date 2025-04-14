#version 330 core

layout (vertices = 25) out;

in VS_OUT {
    vec2 texCoord;
} tcs_in[];

out TCS_OUT {
    vec2 texCoord;
    vec3 position; // 保存原始位置
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
        // 设置外部边界四条边的细分级别
        gl_TessLevelOuter[0] = tessLevel; // 左边
        gl_TessLevelOuter[1] = tessLevel; // 下边
        gl_TessLevelOuter[2] = tessLevel; // 右边
        gl_TessLevelOuter[3] = tessLevel; // 上边
        
        // 设置内部细分级别
        gl_TessLevelInner[0] = tessLevel; // 水平方向
        gl_TessLevelInner[1] = tessLevel; // 垂直方向
    }
} 