#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>                  // glm::vec3, glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::lookAt
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <parser.h>
#include <cstdlib>
#include <shaders.h>

// Define constants
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

GLuint sphereVBO, sphereVAO, sphereEBO;
// GLuint linesVBO, linesVAO, linesEBO;
GLuint shaderProgram;

void initGeometry()
{
    // Generate and bind the VAO
    glGenVertexArrays(1, &sphereVAO);
    // glGenVertexArrays(1, &linesVAO);

    glGenBuffers(1, &sphereVBO);
    // glGenBuffers(1, &linesVBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(GLuint), &sphereIndices[0], GL_STATIC_DRAW);

    // glBindVertexArray(linesVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
    // glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);

    // Setup vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

float targetX = 0.0, targetY = 0.0, targetZ = -1.0;
float cameraX = 0.0, cameraY = 0.0, cameraZ = 5.0;
int xOrigin = -1;
int yOrigin = -1;
int windowWidth = 500;
int windowHeight = 500;
glm::mat4 projectionMatrix;

extern "C"
{
    typedef void (*scrollCallback_t)(float);
    void registerScroll(scrollCallback_t callback);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (yoffset > 0)
    {
        // Zoom in
        cameraZ -= 0.1f;
    }
    else
    {
        // Zoom out
        cameraZ += 0.1f;
    }
}

void mouseButton(GLFWwindow *window, int button, int action, int mods)
{
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    int x = static_cast<int>(xPos);
    int y = static_cast<int>(yPos);
    // Only start motion if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // When the button is released
        if (action == GLFW_RELEASE)
        {
            xOrigin = -1;
            yOrigin = -1;
        }
        else
        {
            xOrigin = x;
            yOrigin = y;
        }
    }
}

void mouseMove(GLFWwindow *window, double xPos, double yPos)
{
    int x = static_cast<int>(xPos);
    int y = static_cast<int>(yPos);
    if (xOrigin >= 0)
    {
        // Update camera's position
        cameraX -= (x - xOrigin) * 0.001f;
        targetX -= (x - xOrigin) * 0.001f;
        cameraY += (y - yOrigin) * 0.001f;
        targetY += (y - yOrigin) * 0.001f;
        xOrigin = x;
        yOrigin = y;
    }
}

glm::vec3 *nodes;

short numModules;
Module *modules;
glm::vec3 *forces;

void print_module(Module *module)
{
    std::cout << "Module:" << module->source << std::endl;
    std::cout << "Dependencies:" << std::endl;
    for (size_t i = 0; i < module->num_dependencies; ++i)
    {
        std::cout << " - " << module->dependencies[i] << std::endl;
    }
}

void initNodes()
{
    modules = load_json_file("file.json", &numModules);

    nodes = static_cast<glm::vec3 *>(malloc(numModules * sizeof(glm::vec3)));
    forces = static_cast<glm::vec3 *>(malloc(numModules * sizeof(glm::vec3)));

    if (modules != NULL)
    {
        int i;
        for (i = 0; i < numModules; i++)
        {
            nodes[i].x = (float)rand() / (float)RAND_MAX * 2 - 1;
            nodes[i].y = (float)rand() / (float)RAND_MAX * 2 - 1;
            nodes[i].z = (float)rand() / (float)RAND_MAX * 2 - 1;
        }
    }
}

void calculateForces()
{
    float idealDistance = 0.1f;
    float speed = 0.1f;

    for (int i = 0; i < numModules; i++)
    {
        forces[i].x = forces[i].y = forces[i].z = 0.0f;

        for (int j = 0; j < numModules; j++)
        {
            if (i == j)
                continue;

            float x = nodes[j].x - nodes[i].x;
            float y = nodes[j].y - nodes[i].y;
            float z = nodes[j].z - nodes[i].z;

            float dist = sqrt(x * x + y * y + z * z);
            if (dist < 0.000001f)
                continue;

            int attraction = 0;
            for (int k = 0; k < modules[i].num_dependencies; k++)
            {
                if (modules[i].dependencies[k] == j)
                {
                    attraction = 1;
                    break;
                }
            }
            float attractPower = fmin(attraction * dist * dist / idealDistance, dist * idealDistance);
            float repulsePower = (1 - attraction) * fmin(idealDistance * idealDistance / (dist * dist), dist * idealDistance);
            float power = attractPower - repulsePower;

            forces[i].x += x / dist * power * speed;
            forces[i].y += y / dist * power * speed;
            forces[i].z += z / dist * power * speed;
        }
    }

    for (int i = 0; i < numModules; i++)
    {
        nodes[i].x += forces[i].x;
        nodes[i].y += forces[i].y;
        nodes[i].z += forces[i].z;
    }
}

void draw(glm::mat4 *modelMatrices)
{
    GLuint modelMatrixBuffer;
    glGenBuffers(1, &modelMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, numModules * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // Now bind the model matrix buffer to your VAO and set up the vertex attributes
    // Note that a mat4 occupies 4 vec4 slots, so we need to set up 4 vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
    for (unsigned int i = 0; i < 4; ++i)
    {
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1); // Tell OpenGL this is an instanced vertex attribute.
    }

    glBindVertexArray(sphereVAO);
    glDrawElementsInstanced(GL_TRIANGLES, sphereVertices.size(), GL_UNSIGNED_INT, 0, numModules);
    glBindVertexArray(0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use glm::lookAt to replace gluLookAt
    glm::mat4 view = glm::lookAt(
        glm::vec3(cameraX, cameraY, cameraZ), // Camera is at (cameraX, cameraY, cameraZ), in World Space
        glm::vec3(targetX, targetY, targetZ), // And looks at the point (targetX, targetY, targetZ), in World Space
        glm::vec3(0.0f, 1.0f, 0.0f)           // Head is up (set to 0,-1,0 to look upside-down)
    );

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

    glm::mat4 *modelMatrices = new glm::mat4[numModules];
    for (int i = 0; i < numModules; i++)
    {
        modelMatrices[i] = glm::translate(glm::mat4(1.0f), nodes[i]);
    }
    draw(modelMatrices);

    // if (modules != NULL)
    // {
    //     for (i = 0; i < numModules; i++)
    //     {
    //         for (j = 0; j < modules[i].num_dependencies; j++)
    //         {
    //             // Assuming the dependencies are represented by their indices
    //             short dep_index = modules[i].dependencies[j];
    //             if (dep_index < numModules)
    //             {
    //                 drawEdge(nodes[i], nodes[dep_index]);
    //             }
    //         }
    //     }
    // }
}

void reshape(GLFWwindow *window, int w, int h)
{
    if (h == 0)
    {
        h = 1;
    }

    float ratio = 1.0 * w / h;

    glViewport(0, 0, w, h);

    projectionMatrix = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
}

void initGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    shaderProgram = createShaderProgramFromFiles("./vertex.glsl", "./fragment.glsl");
}

void update()
{
    calculateForces();
}

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "3D Graph", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, reshape);

    initGL();
    initGeometry();

    while (!glfwWindowShouldClose(window))
    {
        display();
        update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    std::cout << 'c' << std::endl;

    return 0;
}
