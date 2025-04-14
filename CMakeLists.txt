cmake_minimum_required(VERSION 3.10)
project(bezier_surface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 找到必要的包
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# 添加包含目录
include_directories(${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} include)

# 添加源文件
add_executable(bezier_surface 
    src/main.cpp
    src/shader.cpp
)

# 链接库
target_link_libraries(bezier_surface
    OpenGL::GL
    GLEW::GLEW
    glfw
)

# 复制着色器文件到构建目录
file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY textures DESTINATION ${CMAKE_BINARY_DIR}) 