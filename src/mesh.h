#pragma once

#include "glad/glad.h"
#include <vector>

typedef struct {
  float x, y, z;
} Vertex;

class Mesh {
public:
  Mesh(const std::vector<Vertex> &vertices);

private:
  GLuint VAO;
  GLuint VBO;
};
