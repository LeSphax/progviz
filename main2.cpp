#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"

GLuint VBO, VAO, EBO;
GLuint shaderProgram;

#define PI 3.14159265358979323846
#define NODE_RADIUS 0.1
#define EDGE_WIDTH 0.02
#define SPHERE_SLICES 32
#define SPHERE_STACKS 32

std::pair<std::vector<glm::vec3>, std::vector<GLuint>> generateSphere(float radius, unsigned int rings, unsigned int sectors)
{
  std::vector<glm::vec3> vertices;
  std::vector<GLuint> indices;

  float const R = 1. / (float)(rings - 1);
  float const S = 1. / (float)(sectors - 1);

  for (unsigned int r = 0; r < rings; ++r)
  {
    for (unsigned int s = 0; s < sectors; ++s)
    {
      float const y = sin(-(PI / 2) + PI * r * R);
      float const x = cos(2 * PI * s * S) * sin(PI * r * R);
      float const z = sin(2 * PI * s * S) * sin(PI * r * R);

      vertices.push_back(glm::vec3(x, y, z) * radius);

      indices.push_back(r * sectors + s);
      indices.push_back(r * sectors + (s + 1));
      indices.push_back((r + 1) * sectors + (s + 1));

      indices.push_back((r + 1) * sectors + (s + 1));
      indices.push_back((r + 1) * sectors + s);
      indices.push_back(r * sectors + s);
    }
  }

  return std::make_pair(vertices, indices);
}

auto [sphereVertices, sphereIndices] = generateSphere(NODE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);

void initGeometry()
{
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3), &sphereVertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void initGL()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  shaderProgram = createShaderProgramFromFiles("./vertex.glsl", "./fragment.glsl");
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float targetX = 0.0, targetY = 0.0, targetZ = -1.0;
  float cameraX = 0.0, cameraY = 0.0, cameraZ = 5.0;

  glm::mat4 view = glm::lookAt(
      glm::vec3(cameraX, cameraY, cameraZ), // Camera is at (cameraX, cameraY, cameraZ), in World Space
      glm::vec3(targetX, targetY, targetZ), // And looks at the point (targetX, targetY, targetZ), in World Space
      glm::vec3(0.0f, 1.0f, 0.0f)           // Head is up (set to 0,-1,0 to look upside-down)
  );
  glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

  glUseProgram(shaderProgram);
  GLint modelViewLoc = glGetUniformLocation(shaderProgram, "view");
  if (modelViewLoc != -1)
  {
    glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(view));
  }
  else
  {
    // Handle error: there's no such uniform in the shader, or the shader compiler optimized it away.
    std::cerr << "ERROR glGetUniformLocation failed: The parameter view doesn't exist in the shader program\n"
              << std::endl;
    exit(1);
  }
  GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
  if (projLoc != -1)
  {
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
  }
  else
  {
    std::cerr << "ERROR glGetUniformLocation failed: The parameter projection doesn't exist in the shader program\n"
              << std::endl;
    exit(1);
  }

  std::vector<glm::mat4> modelMatrices(1);
  modelMatrices[0] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f, 0.0f, -1.0f)); // left of center
  // modelMatrices[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 0.0f, -1.0f));  // right of center

  GLuint modelMatrixBuffer;
  glGenBuffers(1, &modelMatrixBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
  glBufferData(GL_ARRAY_BUFFER, 1 * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

  // Now bind the model matrix buffer to your VAO and set up the vertex attributes
  // Note that a mat4 occupies 4 vec4 slots, so we need to set up 4 vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
  for (unsigned int i = 0; i < 4; ++i)
  {
    glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
    glEnableVertexAttribArray(1 + i);
    glVertexAttribDivisor(1 + i, 1); // Tell OpenGL this is an instanced vertex attribute.
  }

  glBindVertexArray(VAO);
  // glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
  glDrawElementsInstanced(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0, 1);
  glBindVertexArray(0);
}

int main(int argc, char **argv)
{
  std::cout << "Hello, world!" << std::endl;
  if (!glfwInit())
  {
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // This line is required on Mac
#endif

  GLFWwindow *window = glfwCreateWindow(500, 500, "Simple Quad", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  initGL();
  initGeometry();

  while (!glfwWindowShouldClose(window))
  {
    display();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();

  return 0;
}
