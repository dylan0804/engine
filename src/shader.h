#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
  Shader(const std::string &vertPath, const std::string &fragPath);
  // copy constructor
  // C++ normally auto generates a constructor and it also copies its members
  // (bad) = delete removes this
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  // move constructor
  Shader(Shader &&other) noexcept
      : ID(other.ID), uniforms(std::move(other.uniforms)) {
    other.ID = 0;
  }
  Shader &operator=(Shader &&other) noexcept {
    if (this != &other) {
      glDeleteProgram(ID);
      ID = other.ID;
      uniforms = std::move(other.uniforms);
      other.ID = 0;
    }
    return *this;
  }
  ~Shader();

  void use();
  void setUniformLocation();
  int getUniformLocation(const std::string &name);
  void setMat4(const std::string &name, const glm::mat4 &value);
  void setVec3(const std::string &name, float x, float y, float z);
  unsigned int getId() const;

  void checkShaderErrors(GLuint shader, GLenum pname, GLint *success);
  void checkProgramErrors(GLuint program, GLenum pname, GLint *success);

private:
  unsigned int ID;
  std::unordered_map<std::string, int> uniforms;
};
