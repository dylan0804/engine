#include "glm/ext/scalar_constants.hpp"
#include "glm/trigonometric.hpp"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "stb/stb_image.h"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

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

void generate_vertices(std::vector<float> &vertices) {
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      float x = 0 + i;
      float y = 0.5;
      float z = 0 + j;
      float x1 = 0 + i + 1;
      float z1 = 0 + j + 1;

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z1);
      vertices.push_back(x1);
      vertices.push_back(y);
      vertices.push_back(z);

      vertices.push_back(x1);
      vertices.push_back(y);
      vertices.push_back(z);
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z1);
      vertices.push_back(x1);
      vertices.push_back(y);
      vertices.push_back(z1);
    }
  }
}

void generate_grid_lines(std::vector<float> &lines) {
  float y = 0.505;
  for (int i = 0; i <= 100; i++) {
    float x = (float)i;
    lines.push_back(x);
    lines.push_back(y);
    lines.push_back(0.0f);
    lines.push_back(x);
    lines.push_back(y);
    lines.push_back(100.0f);
  }

  for (int i = 0; i <= 100; i++) {
    float z = (float)i;
    lines.push_back(0.0f);
    lines.push_back(y);
    lines.push_back(z);
    lines.push_back(100.0f);
    lines.push_back(y);
    lines.push_back(z);
  }
}

typedef struct {
  float x, y, z;
} Vertex;

void generate_cylinder(float radius, float height, int segments,
                       std::vector<Vertex> &verts,
                       std::vector<unsigned int> &indices) {
  float half_h = height / 2;

  int verts_per_ring = segments + 1;
  int vert_count = verts_per_ring * 2; // top n bottom

  int vi = 0;

  for (int i = 0; i <= segments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / segments);
    verts.push_back(
        {radius * glm::cos(angle), half_h, radius * glm::sin(angle)});
  }

  for (int i = 0; i <= segments; i++) {
    float angle = 2.f * glm::pi<float>() * ((float)i / segments);
    verts.push_back(
        {radius * glm::cos(angle), -half_h, radius * glm::sin(angle)});
  }

  int max_indices = segments * 6;
  int ii = 0;

  for (int i = 0; i < segments; i++) {
    int topA = i;
    int topB = i + 1;
    int botA = verts_per_ring + i;
    int botB = verts_per_ring + i + 1;

    indices.push_back(topA);
    indices.push_back(botA);
    indices.push_back(topB);
    indices.push_back(topB);
    indices.push_back(botA);
    indices.push_back(botB);
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

  std::vector<float> vertices;
  generate_vertices(vertices);
  std::vector<float> lines;
  generate_grid_lines(lines);

  std::vector<Vertex> verts;
  std::vector<unsigned int> indices;
  generate_cylinder(2.f, 5.f, 24, verts, indices);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0],
               GL_STATIC_DRAW);

  GLuint vertex_buffer2;
  glGenBuffers(1, &vertex_buffer2);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
  glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), &lines[0],
               GL_STATIC_DRAW);

  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  const GLint vpos_location = glGetAttribLocation(program, "aPos");

  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBindVertexArray(vertex_array);
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void *)0);

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  const GLint vpos_location2 = glGetAttribLocation(program, "aPos");

  GLuint vertex_array2;
  glGenVertexArrays(1, &vertex_array2);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
  glBindVertexArray(vertex_array2);
  glEnableVertexAttribArray(vpos_location2);
  glVertexAttribPointer(vpos_location2, 3, GL_FLOAT, GL_FALSE,
                        3 * sizeof(float), (void *)0);

  GLuint vertex_buffer3;
  glGenBuffers(1, &vertex_buffer3);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer3);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0],
               GL_STATIC_DRAW);

  const GLint vpos_location3 = glGetAttribLocation(program, "aPos");
  GLuint vertex_array3;
  glGenVertexArrays(1, &vertex_array3);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer3);
  glBindVertexArray(vertex_array3);
  glEnableVertexAttribArray(vpos_location3);
  glVertexAttribPointer(vpos_location3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)0);

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // int img_width, img_height, chan;
  // unsigned char *data =
  //     stbi_load("./hornet.jpeg", &img_width, &img_height, &chan, 0);

  // unsigned int texture;
  // glGenTextures(1, &texture);
  // glBindTexture(GL_TEXTURE_2D, texture);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
  //                 GL_LINEAR_MIPMAP_LINEAR);
  //
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // if (data) {
  //   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB,
  //                GL_UNSIGNED_BYTE, data);
  //   glGenerateMipmap(GL_TEXTURE_2D);
  // } else {
  //   std::cout << "Failed to load texture" << std::endl;
  // }
  // stbi_image_free(data);

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
    // model = glm::rotate(model, (float)glfwGetTime(),
    //                     glm::vec3(0.f, 2.f, 0.f)); // spin it
    //

    glm::mat4 view =
        glm::lookAt(c->cameraPos, c->cameraPos + c->cameraFront, c->cameraUp);

    glm::mat4 projection =
        glm::perspective(glm::radians(60.0f), ratio, 0.1f, 100.0f);

    glUseProgram(program);
    unsigned int colorLoc = glGetUniformLocation(program, "fillColor");
    glUniform3f(colorLoc, 0.8f, 0.8f, 0.8f);

    unsigned int modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    unsigned int viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    unsigned int projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    colorLoc = glGetUniformLocation(program, "fillColor");
    glUniform3f(colorLoc, 0.f, 0.f, 0.f);

    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vertex_array2);
    glDrawArrays(GL_LINES, 0, lines.size() / 3);

    colorLoc = glGetUniformLocation(program, "fillColor");
    glUniform3f(colorLoc, 0.85f, 0.45f, 0.2f);

    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vertex_array3);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
