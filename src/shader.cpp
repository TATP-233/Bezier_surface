#include "shader.h"

Shader::Shader(const char* vertexPath, const char* tessControlPath, const char* tessEvalPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string tessControlCode;
    std::string tessEvalCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream tcShaderFile;
    std::ifstream teShaderFile;
    std::ifstream fShaderFile;

    // 确保ifstream对象可以抛出异常
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // 打开文件
        vShaderFile.open(vertexPath);
        tcShaderFile.open(tessControlPath);
        teShaderFile.open(tessEvalPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, tcShaderStream, teShaderStream, fShaderStream;
        // 读取文件缓冲到数据流中
        vShaderStream << vShaderFile.rdbuf();
        tcShaderStream << tcShaderFile.rdbuf();
        teShaderStream << teShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // 关闭文件
        vShaderFile.close();
        tcShaderFile.close();
        teShaderFile.close();
        fShaderFile.close();

        // 将数据流转换为字符串
        vertexCode = vShaderStream.str();
        tessControlCode = tcShaderStream.str();
        tessEvalCode = teShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* tcShaderCode = tessControlCode.c_str();
    const char* teShaderCode = tessEvalCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 编译着色器
    unsigned int vertex, tessControl, tessEval, fragment;
    int success;
    char infoLog[512];

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // 曲面细分控制着色器
    tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tessControl, 1, &tcShaderCode, NULL);
    glCompileShader(tessControl);
    checkCompileErrors(tessControl, "TESS_CONTROL");

    // 曲面细分评估着色器
    tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(tessEval, 1, &teShaderCode, NULL);
    glCompileShader(tessEval);
    checkCompileErrors(tessEval, "TESS_EVALUATION");

    // 片段着色器
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // 着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, tessControl);
    glAttachShader(ID, tessEval);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // 删除着色器，它们已经链接到程序中，不再需要
    glDeleteShader(vertex);
    glDeleteShader(tessControl);
    glDeleteShader(tessEval);
    glDeleteShader(fragment);
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
} 