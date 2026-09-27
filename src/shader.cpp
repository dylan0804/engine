#include "shader.h"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <OpenGL/gl.h>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open shader file: " << path << std::endl;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
} // namespace
} // namespace

Shader::Shader(const std::string &vert_path, const std::string &frag_path) {
  std::string vert_src = readFile(vert_path);
  std::string frag_src = readFile(frag_path);

  const char *vert_shader = vert_src.c_str();
  const char *frag_shader = frag_src.c_str();

  GLint success;

  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vert_shader, NULL);
  glCompileShader(vertex_shader);
  checkShaderErrors(vertex_shader, GL_COMPILE_STATUS, &success);

  const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &frag_shader, NULL);
  glCompileShader(fragment_shader);
  checkShaderErrors(fragment_shader, GL_COMPILE_STATUS, &success);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  checkProgramErrors(program, GL_LINK_STATUS, &success);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  ID = program;

  Shader::setUniformLocation();
}
Shader::~Shader() {
  if (ID)
    glDeleteProgram(ID);
}

void Shader::use() { glUseProgram(ID); }

void Shader::setUniformLocation() {
  uniforms["fillColor"] = glGetUniformLocation(ID, "fillColor");
  uniforms["model"] = glGetUniformLocation(ID, "model");
  uniforms["view"] = glGetUniformLocation(ID, "view");
  uniforms["projection"] = glGetUniformLocation(ID, "projection");
}

int Shader::getUniformLocation(const std::string &name) {
  return uniforms[name];
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) {
  glUniformMatrix4fv(Shader::getUniformLocation(name), 1, GL_FALSE,
                     glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, float x, float y, float z) {
  glUniform3f(Shader::getUniformLocation(name), x, y, z);
}

unsigned int Shader::getId() const { return ID; }

void Shader::checkShaderErrors(GLuint shader, GLenum pname, GLint *success) {
  char infoLog[512];
  glGetShaderiv(shader, pname, success);
  if (!*success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "shader compile failed:" << infoLog << std::endl;
  }
}

void Shader::checkProgramErrors(GLuint program, GLenum pname, GLint *success) {
  char infoLog[512];
  glGetProgramiv(program, pname, success);
  if (!*success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "Shader program link failed: " << infoLog << std::endl;
  }
}
