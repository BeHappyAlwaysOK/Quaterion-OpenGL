#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <LearnOpenGL/stb_image.h>
#include <LearnOpenGL/glad.c>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include<LearnOpenGL/shader.h>
#include <iostream>
#include <memory>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
GLFWwindow* initWindow(int width, int height, const char* title);
void processInput(GLFWwindow* window, float deltaTime);
glm::quat myQuatLookAt(glm::vec3 direction, glm::vec3 up);
unsigned int loadTexture(const char *path);

// 自定义四元数摄像机类
class QuaternionCamera {
public:
    // 构造函数
    QuaternionCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),float sensitivity = 0.1f,float speed = 2.5f)
        : position(position),rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),sensitivity(sensitivity),movementSpeed(speed),fov(45.0f){}

    // 处理鼠标输入
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true) {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        // 绕世界Y轴旋转（偏航）
        glm::quat yaw = glm::angleAxis(glm::radians(-xOffset), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = yaw * rotation;

        // 绕本地右轴旋转（俯仰）
        glm::vec3 right = getRightVector();
        glm::quat pitch = glm::angleAxis(glm::radians(-yOffset), right);
        rotation = rotation * pitch;

        // 保持单位四元数
        rotation = glm::normalize(rotation);

        if (constrainPitch) {
            constrainPitchAngle();
        }
    }

    // 处理键盘输入
    void processKeyboard(int key, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        glm::vec3 front = getFrontVector();
        glm::vec3 right = getRightVector();

        switch (key) {
        case GLFW_KEY_W:
            position += front * velocity;
            break;
        case GLFW_KEY_S:
            position -= front * velocity;
            break;
        case GLFW_KEY_A:
            position -= right * velocity;
            break;
        case GLFW_KEY_D:
            position += right * velocity;
            break;
        }
    }

    // 处理滚轮缩放
    void processMouseScroll(float yOffset) {
        fov -= yOffset;
        fov = glm::clamp(fov, 1.0f, 45.0f);
    }

    // 获取视图矩阵
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + getFrontVector(), getUpVector());
    }

    // 获取投影矩阵
    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // 获取各方向向量
    glm::vec3 getFrontVector() const { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
    glm::vec3 getUpVector() const    { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }
    glm::vec3 getRightVector() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }

    float getFOV() const { return fov; }

private:
    glm::vec3 position;
    glm::quat rotation;
    float sensitivity;
    float movementSpeed;
    float fov;

    // 俯仰角限制
    void constrainPitchAngle() {
        glm::vec3 front = getFrontVector();
        glm::vec3 frontXZ = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        float pitch = glm::degrees(glm::asin(front.y));

        const float maxPitch = 89.0f;
        if (pitch > maxPitch || pitch < -maxPitch) {
            front.y = glm::sin(glm::radians(glm::clamp(pitch, -maxPitch, maxPitch)));
            front = glm::normalize(front);

            // 重新构造四元数
            glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::normalize(glm::cross(right, front));
            rotation = myQuatLookAt(front, up);
        }
    }
};

// 全局变量
std::unique_ptr<QuaternionCamera> camera;
bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;


int main() {
    // 初始化窗口
    GLFWwindow* window = initWindow(800, 600, "Quaternion Camera Demo");
    if (!window) return -1;

    // 初始化摄像机
    camera = std::make_unique<QuaternionCamera>();

    // 配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("D:/vscode/OpenGLvscode/shaderfiles/quaterion_Camera_vs.txt","D:/vscode/OpenGLvscode/shaderfiles/quaterion_Camera_fs.txt");

    // 设置顶点数据
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // 设置VBO/VAO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int texture1 = loadTexture("D:/vscode/OpenGLvscode/textures/container.jpg");
    unsigned int texture2 = loadTexture("D:/vscode/OpenGLvscode/textures/smile.png");

    // 配置着色器
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 时间计算
        static float lastFrame = 0.0f;
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入处理
        processInput(window, deltaTime);

        // 清除缓冲
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // 获取矩阵
        glm::mat4 projection = camera->getProjectionMatrix(800.0f / 600.0f);
        glm::mat4 view = camera->getViewMatrix();

        // 设置矩阵uniform
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // 渲染立方体
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 交换缓冲和轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset =  ypos-lastY; // Y轴反转
    lastX = xpos;
    lastY = ypos;

    camera->processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(static_cast<float>(yoffset));
}

GLFWwindow* initWindow(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    return window;
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(GLFW_KEY_D, deltaTime);
}

glm::quat myQuatLookAt(glm::vec3 direction, glm::vec3 up) {
    // 确保方向向量是单位向量
    direction = glm::normalize(direction);

    // 计算右向量
    glm::vec3 right = glm::normalize(glm::cross(direction, up));

    // 重新正交化上向量
    up = glm::normalize(glm::cross(right, direction));

    // 构建旋转矩阵，这里可能需要调整方向，因为相机的视图矩阵是逆旋转
    // 例如，视图矩阵的旋转部分是转置的，所以四元数可能直接由矩阵转置得到
    glm::mat3 rotateMatrix;
    rotateMatrix[0] = right;       // 右向量对应x轴
    rotateMatrix[1] = up;          // 上向量对应y轴
    rotateMatrix[2] = -direction;   // 前向量是-z轴方向

    // 将旋转矩阵转换为四元数
    glm::quat orientation = glm::quat_cast(rotateMatrix);

    return orientation;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
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
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
