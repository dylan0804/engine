#include "camera.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

Camera::Camera(const glm::vec3 &pos, const glm::vec3 &front,
               const glm::vec3 &up, const float yaw)
    : cameraPos(pos), cameraFront(front), cameraUp(up), yaw(yaw) {}

glm::vec3 Camera::getPos() const { return cameraPos; }
glm::vec3 Camera::getFront() const { return cameraFront; }
glm::vec3 Camera::getUp() const { return cameraUp; }
float Camera::getYaw() const { return yaw; }
float Camera::getPitch() const { return pitch; }

void Camera::setUp(const glm::vec3 &up) { cameraUp = up; }
void Camera::setFront(const glm::vec3 &front) { cameraFront = front; }
void Camera::setPos(const glm::vec3 &pos) { cameraPos = pos; }
void Camera::setYaw(float yaw) { this->yaw = yaw; }
void Camera::setPitch(float pitch) { this->pitch = pitch; }

void Camera::processMovement(GLFWwindow *window, float &lastFrame,
                             float &deltaTime) {
  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  float speed = 2.5f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    setPos(getPos() + (speed * getFront()));
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    setPos(getPos() - (speed * getFront()));
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    setPos(getPos() -
           (speed * glm::normalize(glm::cross(getFront(), getUp()))));
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    setPos(getPos() +
           (speed * glm::normalize(glm::cross(getFront(), getUp()))));
}
