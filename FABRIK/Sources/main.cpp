#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <string>

using namespace std;

struct Point {
    double x;
    double y;

    Point operator+(Point& operand) {
        return { x + operand.x, y + operand.y };
    }

    Point operator-(Point& operand) {
        return { x - operand.x, y - operand.y };
    }

    Point operator*(double operand) {
        return { x * operand, y * operand };
    }
};

// The length of each bone
int length = 80;
// The repeat times
int repeats = 8;
// The start location of bones
Point startPoint = { 320.0, 60.0 };
// The position of each bone's start and end
vector<Point> points = {
      { startPoint.x, startPoint.y }
    , { startPoint.x, startPoint.y + length }
    , { startPoint.x, startPoint.y + length * 2 }
    , { startPoint.x, startPoint.y + length * 3 }
    , { startPoint.x, startPoint.y + length * 4 }
};

// Get the distance between points
double magnitude(Point p1, Point p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

// Drag the end position to the tail of bone
void dragEndPos(Point start, Point* end) {
    double mag = magnitude(start, *end);
    double scale = length / mag;
    Point toward = (*end - start) * scale;
    Point dest = (start + toward);
    end->x = dest.x;
    end->y = dest.y;
}

// Iterates each bone and correct position
void iterate(double cursor_x, double cursor_y) {
    for (int r = 0; r < repeats; r++) {
        //backward
        vector<Point>::iterator end = (points.end() - 1);
        end->x = cursor_x;
        end->y = cursor_y;

        for (vector<Point>::iterator iter = points.end() - 1; iter != points.begin(); --iter) {
            dragEndPos(*iter, &*(iter - 1));
        }
        
        //forward
        vector<Point>::iterator front = points.begin();
        front->x = startPoint.x;
        front->y = startPoint.y;
        
        for (vector<Point>::iterator iter = points.begin(); iter + 1 != points.end(); ++iter) {
            dragEndPos(*iter, &*(iter + 1));
        }
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    int width = 640;
    int height = 480;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "FABRIK simulation", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        
        double x_cursor, y_cursor;

        glfwGetCursorPos(window, &x_cursor, &y_cursor);
        x_cursor = clamp(x_cursor, 0.0, (double)width);
        y_cursor = clamp(height - y_cursor, 0.0, (double)height);
        
        //Apply fabrik
        iterate(x_cursor, y_cursor);

        //Draw bones
        double widthHalf = width / 2;
        double heightHalf = height / 2;

        glPointSize(10.0);
        glBegin(GL_POINTS);

        for (int i = 0; i < points.size(); i++) {
            Point p = points[i];
            double xInScreen = (p.x - widthHalf) / widthHalf;
            double yInScreen = (p.y - heightHalf) / heightHalf;

            glVertex2d(xInScreen, yInScreen);
        }

        glEnd();
        glFinish();

        glLineWidth(2.0);
        glBegin(GL_LINE_STRIP);

        for (int i = 0; i < points.size(); i++) {
            Point p = points[i];
            double xInScreen = (p.x - widthHalf) / widthHalf;
            double yInScreen = (p.y - heightHalf) / heightHalf;
            glVertex2d(xInScreen, yInScreen);
        }

        glEnd();
        glFinish();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}