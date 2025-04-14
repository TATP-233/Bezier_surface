#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <iomanip>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"

// 窗口尺寸
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 贝塞尔曲面控制
float tessellationLevel = 2.0f;
bool wireframeMode = false;

// 相机控制变量
glm::vec3 cameraPos = glm::vec3(-2.0f, 2.0f, 0.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraPitch = -30.0f;
float cameraYaw = -90.0f;
float cameraDistance = 3.0f;
float cameraZoom = 45.0f;

// 鼠标控制变量
bool leftMousePressed = false;
bool rightMousePressed = false;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float mouseSensitivity = 0.1f;
bool firstMouse = true;

// 更新相机位置
void updateCamera() {
    // 计算相机位置
    float camX = cameraDistance * cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    float camY = cameraDistance * sin(glm::radians(cameraPitch));
    float camZ = cameraDistance * sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraPos = cameraTarget + glm::vec3(camX, camY, camZ);
}

// 回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 键盘回调函数修改
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // 增加细分级别
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        tessellationLevel = std::min(50.0f, tessellationLevel + 1.0f);
        std::cout << "细分级别: " << tessellationLevel << std::endl;
    }
    
    // 减少细分级别
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        tessellationLevel = std::max(1.0f, tessellationLevel - 1.0f);
        std::cout << "细分级别: " << tessellationLevel << std::endl;
    }
    
    // 切换线框模式
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        wireframeMode = !wireframeMode;
        std::cout << "线框模式: " << (wireframeMode ? "开启" : "关闭") << std::endl;
    }
}

// 鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    // 左键旋转
    if (leftMousePressed) {
        cameraYaw += xoffset;
        cameraPitch += yoffset;

        // 限制俯仰角，防止翻转
        if (cameraPitch > 89.0f)
            cameraPitch = 89.0f;
        if (cameraPitch < -89.0f)
            cameraPitch = -89.0f;

        updateCamera();
    }

    // 右键平移
    if (rightMousePressed) {
        float panSpeed = 0.01f * cameraDistance;
        glm::vec3 front = glm::normalize(cameraTarget - cameraPos);
        glm::vec3 right = glm::normalize(glm::cross(front, cameraUp));
        cameraTarget -= right * xoffset * panSpeed;
        cameraTarget += glm::normalize(glm::cross(right, front)) * yoffset * panSpeed;
        updateCamera();
    }
}

// 鼠标按键回调函数
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            leftMousePressed = true;
        else if (action == GLFW_RELEASE)
            leftMousePressed = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS)
            rightMousePressed = true;
        else if (action == GLFW_RELEASE)
            rightMousePressed = false;
    }
}

// 鼠标滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraDistance -= static_cast<float>(yoffset) * 0.2f;
    // 限制距离，防止过近或过远
    if (cameraDistance < 1.0f)
        cameraDistance = 1.0f;
    if (cameraDistance > 10.0f)
        cameraDistance = 10.0f;
    updateCamera();
}

// 加载纹理，确保是彩色的
unsigned int loadTexture(const char *path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        
        // 确认纹理加载成功及其格式
        std::cout << "纹理加载成功: " << path << ", 格式: ";
        if (format == GL_RGB || format == GL_RGBA)
            std::cout << "彩色";
        else
            std::cout << "灰度";
        std::cout << ", 分量数: " << nrComponents << std::endl;
    }
    else {
        std::cout << "纹理加载失败: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

int main() {
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建GLFW窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "贝塞尔曲面", NULL, NULL);
    if (window == NULL) {
        std::cout << "创建GLFW窗口失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "初始化GLEW失败" << std::endl;
        return -1;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 编译着色器程序
    Shader bezierShader("shaders/bezier.vert", "shaders/bezier.tesc", "shaders/bezier.tese", "shaders/bezier.frag");
    
    // 创建一个简单的着色器用于绘制控制点
    const char* pointVertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";
    
    const char* pointFragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(0.0, 0.5, 1.0, 1.0);\n" // 明亮的蓝色
        "}\0";
    
    // 编译控制点着色器
    unsigned int pointVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pointVertexShader, 1, &pointVertexShaderSource, NULL);
    glCompileShader(pointVertexShader);
    
    unsigned int pointFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pointFragmentShader, 1, &pointFragmentShaderSource, NULL);
    glCompileShader(pointFragmentShader);
    
    // 创建控制点着色器程序
    unsigned int pointShaderProgram = glCreateProgram();
    glAttachShader(pointShaderProgram, pointVertexShader);
    glAttachShader(pointShaderProgram, pointFragmentShader);
    glLinkProgram(pointShaderProgram);
    
    // 删除着色器，它们已经链接到程序中，不再需要
    glDeleteShader(pointVertexShader);
    glDeleteShader(pointFragmentShader);

    // 随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> heightDist(-1.f, 1.f);

    // 创建5x5控制点网格
    std::vector<float> controlPoints;
    
    // 控制点在平面上的分布
    float step = 0.5f; // 控制点之间的间距
    float startX = -1.0f;
    float startZ = -1.0f;
    
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            // 位置 (x,y,z) - 确保均匀分布
            float x = startX + j * step;
            float z = startZ + i * step;
            
            // 随机高度
            float y;
            y = heightDist(gen);
            
            controlPoints.push_back(x);
            controlPoints.push_back(0.0);
            controlPoints.push_back(z);
            
            // 纹理坐标 (u,v) - 精确对应
            controlPoints.push_back(j / 4.0f);
            controlPoints.push_back(i / 4.0f);
        }
    }
    
    // 配置主要曲面VAO/VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(float), controlPoints.data(), GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // 额外创建控制点显示用VAO (用同一个VBO)
    unsigned int controlPointsVAO;
    glGenVertexArrays(1, &controlPointsVAO);
    
    glBindVertexArray(controlPointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // 重用同一个VBO
    
    // 只需要位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 加载纹理，确保是彩色的
    unsigned int texture = loadTexture("textures/surface.jpg");

    // 初始化相机位置
    updateCamera();

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 清空颜色缓冲和深度缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置渲染模式
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // 激活着色器
        bezierShader.use();

        // 设置变换矩阵
        glm::mat4 model = glm::mat4(1.0f);
        
        // 使用相机视图矩阵
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        
        glm::mat4 projection = glm::perspective(glm::radians(cameraZoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        bezierShader.setMat4("model", model);
        bezierShader.setMat4("view", view);
        bezierShader.setMat4("projection", projection);
        bezierShader.setFloat("tessLevel", tessellationLevel+2);
        bezierShader.setBool("wireframe", wireframeMode);
        bezierShader.setBool("showControlPoints", false); // 默认不显示控制点
        
        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        bezierShader.setInt("textureSurface", 0);

        // 渲染贝塞尔曲面
        glBindVertexArray(VAO);
        glPatchParameteri(GL_PATCH_VERTICES, 25);
        glDrawArrays(GL_PATCHES, 0, 25);
        

        if (wireframeMode) {
            // 渲染控制点
            glPointSize(11.0f);
            
            // 禁用深度测试，确保控制点总是可见
            glDisable(GL_DEPTH_TEST);
            
            // 切换到点模式
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            
            // 使用单独的着色器程序绘制控制点
            glUseProgram(pointShaderProgram);
            
            // 设置变换矩阵
            glUniformMatrix4fv(glGetUniformLocation(pointShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(pointShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(pointShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            
            // 绑定控制点VAO
            glBindVertexArray(controlPointsVAO);
            
            // 绘制所有25个控制点
            glDrawArrays(GL_POINTS, 0, 25);
            
            // 恢复设置
            glUseProgram(bezierShader.ID);
            
            // 恢复深度测试
            glEnable(GL_DEPTH_TEST);
        }
        
        // 交换缓冲并检查事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &controlPointsVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture);
    glDeleteProgram(pointShaderProgram); // 删除控制点着色器程序

    // 终止GLFW
    glfwTerminate();
    return 0;
} 