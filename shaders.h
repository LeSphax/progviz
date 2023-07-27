#pragma once
#include <string>
#include <OpenGL/gl3.h>

// Load shader source code from a file
std::string loadShaderSource(const std::string &filename);

// Compile a shader from its source code
GLuint compileShader(GLenum type, const std::string &source);

// Link shaders into a program
GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader);

// Utility function to create a program from shader files
GLuint createShaderProgramFromFiles(const std::string &vertexShaderFile, const std::string &fragmentShaderFile);
