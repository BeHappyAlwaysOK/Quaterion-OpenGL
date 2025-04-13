#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

//自己定义的LookAt函数，适用于四元数下的相机
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

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class QuaternionCamera{
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

public:
    //构造函数
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
        glm::quat pitch = glm::angleAxis(glm::radians(yOffset), right);
        rotation =  pitch*rotation;

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

    // 获取视图矩阵
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + getFrontVector(), getUpVector());
    }

    // 获取投影矩阵
    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // 获取各方向向量
    glm::vec3 getFrontVector() const { return rotation * glm::vec3(0.0f, -1.0f, 0.0f); }
    glm::vec3 getUpVector() const    { return rotation * glm::vec3(0.0f, 0.0f, 1.0f); }
    glm::vec3 getRightVector() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
    glm::vec3 getPosition() const{return this->position;}
    float getFOV() const { return fov; }
};
#endif