#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "mesh.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ostream>
#include <vector>

typedef struct {
  glm::vec3 cameraPos;
  glm::vec3 cameraUp;
  glm::vec3 cameraFront;
  float yaw;
  float pitch;
} Camera;

static const char *vertex_shader_text =
    "#version 330\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    "void main() {\n"
    "     gl_Position = projection * view * model * vec4(aPos, 1.0);"
    "}\n";

static const char *fragment_shader_text =
    "#version 330\n"
    "uniform vec3 fillColor;"
    "out vec4 FragColor;"
    "void main()\n"
    "{\n"
    "     FragColor = vec4(fillColor, 1.0);"
    "}\n";

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);

  Camera *c = (Camera *)glfwGetWindowUserPointer(window);

  float currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  float speed = 2.5f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    c->cameraPos += speed * c->cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    c->cameraPos -= speed * c->cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    c->cameraPos -=
        glm::normalize(glm::cross(c->cameraFront, c->cameraUp)) * speed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    c->cameraPos +=
        glm::normalize(glm::cross(c->cameraFront, c->cameraUp)) * speed;
}

float lastX, lastY;
bool firstMouse = true;
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  Camera *c = (Camera *)glfwGetWindowUserPointer(window);
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  c->yaw += xoffset * 0.1;
  c->pitch += yoffset * 0.1;

  if (c->pitch > 89.f)
    c->pitch = 89.f;
  if (c->pitch < -89.f)
    c->pitch = -89.f;

  glm::vec3 direction;
  direction.x = cos(glm::radians(c->yaw)) * cos(glm::radians(c->pitch));
  direction.y = sin(glm::radians(c->pitch));
  direction.z = sin(glm::radians(c->yaw)) * cos(glm::radians(c->pitch));
  c->cameraFront = glm::normalize(direction);
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    std::cout << "clicked" << std::endl;
}

void generate_vertices(std::vector<Vertex> &vertices,
                       std::vector<unsigned int> &ebo) {
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      int start = vertices.size();
      float x = 0 + i;
      float y = 0.5;
      float z = 0 + j;
      float x1 = 0 + i + 1;
      float z1 = 0 + j + 1;

      vertices.push_back({x, y, z});   // botA
      vertices.push_back({x, y, z1});  // topA
      vertices.push_back({x1, y, z});  // botB
      vertices.push_back({x1, y, z1}); // topB

      ebo.push_back(start);
      ebo.push_back(start + 1);
      ebo.push_back(start + 3);
      ebo.push_back(start + 3);
      ebo.push_back(start + 2);
      ebo.push_back(start);
    }
  }
}

void generate_cylinder(float radius, float height, int longSegments,
                       int latSegments, std::vector<Vertex> &verts,
                       std::vector<unsigned int> &indices) {
  float half_h = height / 2;
  int verts_per_ring = longSegments + 1;
  int vert_count = verts_per_ring * 2; // top n bottom

  // top hemisphere
  for (int i = 0; i <= latSegments; i++) {
    float theta = (glm::pi<float>() / 2) * ((float)i / latSegments);
    float ringRadius = radius * sin(theta);

    for (int j = 0; j <= longSegments; j++) {
      float angle = 2.f * glm::pi<float>() * ((float)j / longSegments);
      float y = half_h + (radius * cos(theta));
      verts.push_back({ringRadius * cos(angle), y, ringRadius * sin(angle)});
    }
  }

  for (int lat = 0; lat < latSegments; lat++) {
    int ringA = lat * (longSegments + 1);
    int ringB = (lat + 1) * (longSegments + 1);

    for (int j = 0; j < longSegments; j++) {
      int topA = ringA + j;
      int topB = ringA + j + 1;
      int botA = ringB + j;
      int botB = ringB + j + 1;

      indices.push_back(topA);
      indices.push_back(botA);
      indices.push_back(topB);
      indices.push_back(topB);
      indices.push_back(botA);
      indices.push_back(botB);
    }
  }

  int wall_start = verts.size();
  for (int i = 0; i <= longSegments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / longSegments);
    verts.push_back({radius * cos(angle), half_h, radius * sin(angle)});
  }

  for (int i = 0; i <= longSegments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / longSegments);
    verts.push_back({radius * cos(angle), -half_h, radius * sin(angle)});
  }

  for (int i = 0; i <= longSegments; i++) {
    int topA = wall_start + i;
    int topB = wall_start + i + 1;
    int botA = verts_per_ring + wall_start + i;
    int botB = verts_per_ring + wall_start + i + 1;

    indices.push_back(topA);
    indices.push_back(botA);
    indices.push_back(topB);
    indices.push_back(topB);
    indices.push_back(botA);
    indices.push_back(botB);
  }

  int bot_start = verts.size();
  // bottom hemisphere
  for (int i = 0; i <= latSegments; i++) {
    float theta = (glm::pi<float>() / 2) * ((float)i / latSegments);
    float ringRadius = radius * sin(theta);

    for (int j = 0; j <= longSegments; j++) {
      float angle = 2.f * glm::pi<float>() * ((float)j / longSegments);
      float y = -half_h - (radius * cos(theta));
      verts.push_back({ringRadius * cos(angle), y, ringRadius * sin(angle)});
    }
  }

  for (int lat = 0; lat < latSegments; lat++) {
    int ringA = bot_start + (lat * verts_per_ring);
    int ringB = bot_start + ((lat + 1) * verts_per_ring);

    for (int j = 0; j < longSegments; j++) {
      int topA = ringA + j;
      int topB = ringA + j + 1;
      int botA = ringB + j;
      int botB = ringB + j + 1;

      indices.push_back(topA);
      indices.push_back(botA);
      indices.push_back(topB);
      indices.push_back(topB);
      indices.push_back(botA);
      indices.push_back(botB);
    }
  }
}

int main(void) {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  Camera camera = {};
  camera.yaw = -90.f;
  glfwSetWindowUserPointer(window, &camera);

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // if (glfwRawMouseMotionSupported())
  // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> ebo;
  generate_vertices(vertices, ebo);

  std::vector<Vertex> verts;
  std::vector<unsigned int> indices;
  generate_cylinder(0.4f, 1.0f, 32, 8, verts, indices);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);

  Shader shader = Shader("src/vert.glsl", "src/frag.glsl");
  unsigned int program = shader.getId();
  const GLint vpos_location = glGetAttribLocation(program, "aPos");

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBindVertexArray(vertex_array);
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)0);

  GLuint ebo2;
  glGenBuffers(1, &ebo2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo.size() * sizeof(unsigned int),
               &ebo[0], GL_STATIC_DRAW);

  GLuint vertex_buffer3;
  glGenBuffers(1, &vertex_buffer3);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer3);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0],
               GL_STATIC_DRAW);

  GLuint vertex_array3;
  glGenVertexArrays(1, &vertex_array3);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer3);
  glBindVertexArray(vertex_array3);
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)0);

  GLuint ebo1;
  glGenBuffers(1, &ebo1);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  Camera *c = (Camera *)glfwGetWindowUserPointer(window);
  c->cameraPos = glm::vec3(0.f, 0.f, 3.f);
  c->cameraFront = glm::vec3(0.f, 0.f, -1.f);
  c->cameraUp = glm::vec3(0.f, 1.f, 0.f);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float ratio = width / (float)height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view =
        glm::lookAt(c->cameraPos, c->cameraPos + c->cameraFront, c->cameraUp);
    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), ratio, 0.1f, 100.0f);

    shader.use();
    shader.setVec3("fillColor", 0.8f, 0.8f, 0.8f);
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(vertex_array);
    glDrawElements(GL_TRIANGLES, ebo.size(), GL_UNSIGNED_INT, 0);

    shader.setVec3("fillColor", 0.85f, 0.45f, 0.2f);
    model = glm::translate(model, glm::vec3(0.f, 1.4f, 100.f));
    shader.setMat4("model", model);

    glBindVertexArray(vertex_array3);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
