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

  void draw() const;

private:
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  GLsizei indexCount;
};
