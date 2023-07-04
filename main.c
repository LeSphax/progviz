#include <OpenGL/gl.h>    // or <OpenGL/gl3.h>
#include <GLUT/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


// Define constants
#define PI 3.14159265358979323846
#define NODE_RADIUS 0.1
#define NUM_NODES 10
#define EDGE_WIDTH 0.02
#define SPHERE_SLICES 32
#define SPHERE_STACKS 32

float lx = 0.0, ly = 0.0, lz = -1.0;
float x = 0.0, y= 0.0, z = 5.0;
int xOrigin = -1;
int yOrigin = -1;


void mouseButton(int button, int state, int x, int y) {
    // Only start motion if the left button is pressed
    if (button == GLUT_LEFT_BUTTON) {
        // When the button is released
        if (state == GLUT_UP) {
            xOrigin = -1;
            yOrigin = -1;
        }
        else { // state = GLUT_DOWN
            xOrigin = x;
            yOrigin = y;
        }
    }
    glutPostRedisplay();
}

void mouseMove(int x, int y) {
    if (xOrigin >= 0) {
        // Update camera's position
        lx -= (x - xOrigin) * 0.001f;
        ly += (y - yOrigin) * 0.001f;
    }
    glutPostRedisplay();
}

typedef struct {
    float x, y, z;
} Node;

Node nodes[NUM_NODES];

void initNodes() {
    int i;
    for (i = 0; i < NUM_NODES; i++) {
        nodes[i].x = (float)rand()/(float)RAND_MAX * 2 - 1;
        nodes[i].y = (float)rand()/(float)RAND_MAX * 2 - 1;
        nodes[i].z = (float)rand()/(float)RAND_MAX * 2 - 1;
    }
}

void drawNode(Node node) {
    glPushMatrix();
    glTranslatef(node.x, node.y, node.z);
    glutSolidSphere(NODE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);
    glPopMatrix();
}

void drawEdge(Node n1, Node n2) {
    glPushMatrix();
    glLineWidth(EDGE_WIDTH);
    glBegin(GL_LINES);
        glVertex3f(n1.x, n1.y, n1.z);
        glVertex3f(n2.x, n2.y, n2.z);
    glEnd();
    glPopMatrix();
}

void display() {
    int i, j;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    printf("x: %f, y: %f, z: %f\n", x, y, z);
    printf("lx: %f, ly: %f, lz: %f\n", lx, ly, lz);

    gluLookAt(x, y, z,
        lx, ly, lz,
        0.0f, 1.0f, 0.0f);
    
    for (i = 0; i < NUM_NODES; i++) {
        drawNode(nodes[i]);
    }
    
    for (i = 0; i < NUM_NODES; i++) {
        for (j = i + 1; j < NUM_NODES; j++) {
            drawEdge(nodes[i], nodes[j]);
        }
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) {
        h = 1;
    }

    float ratio = 1.0 * w / h;

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    glViewport(0, 0, w, h);

    gluPerspective(45, ratio, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char **argv) {
    initNodes();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("3D Graph");
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);

    
    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}
