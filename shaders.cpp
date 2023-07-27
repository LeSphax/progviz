#include <fstream>
#include <sstream>
#include <string>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <iostream>
#include "shaders.h"

std::string loadShaderSource(const std::string &filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open shader file " + filename);
  }

  std::stringstream stream;
  stream << file.rdbuf();
  return stream.str();
}

GLuint compileShader(GLenum type, const std::string &source)
{
  GLuint shader = glCreateShader(type);
  const char *src = source.c_str();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  // Check for errors
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
    std::string typeName = (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    std::cerr << "ERROR::SHADER::" << typeName << "::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  return shader;
}

GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  // Check for errors
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
    exit(1);
  }

  return program;
}

GLuint createShaderProgramFromFiles(const std::string &vertexShaderFile, const std::string &fragmentShaderFile)
{
  std::string vertexSource = loadShaderSource(vertexShaderFile);
  std::string fragmentSource = loadShaderSource(fragmentShaderFile);

  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);

  // Don't forget to delete the shaders as they're linked into program now and no longer necessary
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}
