#pragma once

#include "glad/glad.h"
#include <vector>

typedef struct {
  float x, y, z;
} Vertex;

class Mesh {
public:
  Mesh() = default;
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<unsigned int> &indices);

  Mesh(const Mesh &other) = delete;
  Mesh &operator=(const Mesh &other) = delete;

  Mesh(Mesh &&other) noexcept
      : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
        indexCount(other.indexCount) {
    other.VAO = 0;
    other.VBO = 0;
    other.EBO = 0;
  }
  Mesh &operator=(Mesh &&other) noexcept {
    if (this != &other) {
      glDeleteVertexArrays(1, &VAO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &EBO);
      VAO = other.VAO;
      VBO = other.VBO;
      EBO = other.EBO;
      indexCount = other.indexCount;
      other.VAO = 0;
      other.VBO = 0;
      other.EBO = 0;
    }

    return *this;
  }
  ~Mesh();

  void draw() const;

private:
  GLuint VAO = 0;
  GLuint VBO = 0;
  GLuint EBO = 0;
  GLsizei indexCount = 0;
};
