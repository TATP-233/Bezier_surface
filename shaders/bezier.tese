#version 330 core

layout(quads, equal_spacing, ccw) in;

in TCS_OUT {
    vec2 texCoord;
    vec3 position; // 从TCS接收原始位置
} tes_in[];

out TES_OUT {
    vec2 texCoord;
} tes_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// 伯恩斯坦多项式
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

void main() {
    // 获取参数 u 和 v
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    
    vec3 pos = bezierSurface(u, v);

    // 应用变换
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    // 纹理坐标 - 仍然限制在[0,1]范围内
    tes_out.texCoord = vec2(clamp(u, 0.0, 1.0), clamp(v, 0.0, 1.0));
} 