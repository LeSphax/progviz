#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <parser.h>
#include <cstdlib>

// Define constants
#define PI 3.14159265358979323846
#define NODE_RADIUS 0.1
#define EDGE_WIDTH 0.02
#define SPHERE_SLICES 32
#define SPHERE_STACKS 32

float targetX = 0.0, targetY = 0.0, targetZ = -1.0;
float cameraX = 0.0, cameraY = 0.0, cameraZ = 5.0;
int xOrigin = -1;
int yOrigin = -1;

extern "C"
{
    typedef void (*scrollCallback_t)(float);
    void registerScroll(scrollCallback_t callback);
}

void scrollCallback(float delta)
{
    if (delta > 0)
    {
        // Zoom in
        cameraZ -= 0.1f;
    }
    else
    {
        // Zoom out
        cameraZ += 0.1f;
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
    // Only start motion if the left button is pressed
    if (button == GLUT_LEFT_BUTTON)
    {
        // When the button is released
        if (state == GLUT_UP)
        {
            xOrigin = -1;
            yOrigin = -1;
        }
        else
        { // state = GLUT_DOWN
            xOrigin = x;
            yOrigin = y;
        }
    }
    glutPostRedisplay();
}

void mouseMove(int x, int y)
{
    if (xOrigin >= 0)
    {
        // Update camera's position
        cameraX -= (x - xOrigin) * 0.001f;
        targetX -= (x - xOrigin) * 0.001f;
        cameraY += (y - yOrigin) * 0.001f;
        targetY += (y - yOrigin) * 0.001f;
    }
    glutPostRedisplay();
}

typedef struct
{
    float x, y, z;
} Vector3;

Vector3 *nodes;

short num_modules;
Module *modules;
Vector3 *forces;

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
    modules = load_json_file("file.json", &num_modules);

    nodes = static_cast<Vector3 *>(malloc(num_modules * sizeof(Vector3)));
    forces = static_cast<Vector3 *>(malloc(num_modules * sizeof(Vector3)));

    if (modules != NULL)
    {
        int i;
        for (i = 0; i < num_modules; i++)
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

    for (int i = 0; i < num_modules; i++)
    {
        forces[i].x = forces[i].y = forces[i].z = 0.0f;

        for (int j = 0; j < num_modules; j++)
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

    for (int i = 0; i < num_modules; i++)
    {
        nodes[i].x += forces[i].x;
        nodes[i].y += forces[i].y;
        nodes[i].z += forces[i].z;
    }
}

void drawNode(Vector3 node)
{
    glPushMatrix();
    glTranslatef(node.x, node.y, node.z);
    glutSolidSphere(NODE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
}

void drawEdge(Vector3 n1, Vector3 n2)
{
    glPushMatrix();
    glLineWidth(EDGE_WIDTH);
    glBegin(GL_LINES);
    glVertex3f(n1.x, n1.y, n1.z);
    glVertex3f(n2.x, n2.y, n2.z);
    glEnd();
    glPopMatrix();
}

void display()
{
    int i, j;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(cameraX, cameraY, cameraZ,
              targetX, targetY, targetZ,
              0.0f, 1.0f, 0.0f);

    for (i = 0; i < num_modules; i++)
    {
        drawNode(nodes[i]);
    }

    if (modules != NULL)
    {
        for (i = 0; i < num_modules; i++)
        {
            for (j = 0; j < modules[i].num_dependencies; j++)
            {
                // Assuming the dependencies are represented by their indices
                short dep_index = modules[i].dependencies[j];
                if (dep_index < num_modules)
                {
                    drawEdge(nodes[i], nodes[dep_index]);
                }
            }
        }
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0)
    {
        h = 1;
    }

    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(45, ratio, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
}

void initGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void update()
{
    calculateForces();
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    std::cout << "aa" << std::endl;
    initNodes();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("3D Graph");
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    std::cout << 'a' << std::endl;
    registerScroll(scrollCallback);
    std::cout << 'b' << std::endl;

    initGL();

    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutReshapeFunc(reshape);

    glutMainLoop();
    std::cout << 'c' << std::endl;

    return 0;
}
