#pragma once

#include "glm/glm.hpp"

class Camera {
public:
  Camera(const glm::vec3 &pos, const glm::vec3 &front, const glm::vec3 &up,
         const float yaw);

  void setFront(const glm::vec3 &front);
  void setPos(const glm::vec3 &pos);
  void setUp(const glm::vec3 &up);
  void setYaw(float yaw);
  void setPitch(float pitch);
  glm::vec3 getPos() const;
  glm::vec3 getFront() const;
  glm::vec3 getUp() const;
  float getYaw() const;
  float getPitch() const;

private:
  glm::vec3 cameraPos;
  glm::vec3 cameraUp;
  glm::vec3 cameraFront;
  float yaw;
  float pitch;
};
