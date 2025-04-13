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

//�Լ������LookAt��������������Ԫ���µ����
glm::quat myQuatLookAt(glm::vec3 direction, glm::vec3 up) {
    // ȷ�����������ǵ�λ����
    direction = glm::normalize(direction);

    // ����������
    glm::vec3 right = glm::normalize(glm::cross(direction, up));

    // ����������������
    up = glm::normalize(glm::cross(right, direction));

    // ������ת�������������Ҫ����������Ϊ�������ͼ����������ת
    // ���磬��ͼ�������ת������ת�õģ�������Ԫ������ֱ���ɾ���ת�õõ�
    glm::mat3 rotateMatrix;
    rotateMatrix[0] = right;       // ��������Ӧx��
    rotateMatrix[1] = up;          // ��������Ӧy��
    rotateMatrix[2] = -direction;   // ǰ������-z�᷽��

    // ����ת����ת��Ϊ��Ԫ��
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

    // ����������
    void constrainPitchAngle() {
        glm::vec3 front = getFrontVector();
        glm::vec3 frontXZ = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        float pitch = glm::degrees(glm::asin(front.y));

        const float maxPitch = 89.0f;
        if (pitch > maxPitch || pitch < -maxPitch) {
            front.y = glm::sin(glm::radians(glm::clamp(pitch, -maxPitch, maxPitch)));
            front = glm::normalize(front);

            // ���¹�����Ԫ��
            glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::normalize(glm::cross(right, front));
            rotation = myQuatLookAt(front, up);
        }
    }

public:
    //���캯��
    QuaternionCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),float sensitivity = 0.1f,float speed = 2.5f)
        : position(position),rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),sensitivity(sensitivity),movementSpeed(speed),fov(45.0f){}

    // �����������
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true) {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        // ������Y����ת��ƫ����
        glm::quat yaw = glm::angleAxis(glm::radians(-xOffset), glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = yaw * rotation;

        // �Ʊ���������ת��������
        glm::vec3 right = getRightVector();
        glm::quat pitch = glm::angleAxis(glm::radians(yOffset), right);
        rotation =  pitch*rotation;

        // ���ֵ�λ��Ԫ��
        rotation = glm::normalize(rotation);

        if (constrainPitch) {
            constrainPitchAngle();
        }
    }

    // �����������
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

    // ��ȡ��ͼ����
    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + getFrontVector(), getUpVector());
    }

    // ��ȡͶӰ����
    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    // ��ȡ����������
    glm::vec3 getFrontVector() const { return rotation * glm::vec3(0.0f, -1.0f, 0.0f); }
    glm::vec3 getUpVector() const    { return rotation * glm::vec3(0.0f, 0.0f, 1.0f); }
    glm::vec3 getRightVector() const { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
    glm::vec3 getPosition() const{return this->position;}
    float getFOV() const { return fov; }
};
#endif