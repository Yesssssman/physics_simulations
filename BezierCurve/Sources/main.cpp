#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <math.h>
#include <string>

using namespace std;

//Screen width, height
int width = 640;
int height = 480;
int interpolationCount = 50;

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

// The position of each control point
vector<Point> points = {
      {       100, 100 }
    , { 100 + 110, 400 }
    , { 100 + 220, 100 }
    , { 100 + 330, 400 }
    , { 100 + 440, 100 }
};

Point* draggingPoint = NULL;

// The bezier coefficient1
vector<Point> coefficients1;

// The bezier coefficient2
vector<Point> coefficients2;

void fill_coefficients(vector<Point> points) {
    int size = points.size() - 1;
    vector<Point> result_term(size);

    vector<Point>(size).swap(coefficients1);
    vector<Point>(size).swap(coefficients2);

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            result_term[i] = {     points[i].x + 2 * points[i + 1].x,     points[i].y + 2 * points[i + 1].y };
        } else if (i == size - 1) {
            result_term[i] = { 8 * points[i].x +     points[i + 1].x, 8 * points[i].y +     points[i + 1].y };
        } else {
            result_term[i] = { 4 * points[i].x + 2 * points[i + 1].x, 4 * points[i].y + 2 * points[i + 1].y };
        }
    }
    
    vector<double> modified_matrix_term(size - 1);
    vector<Point> modified_result_term(size);

    modified_matrix_term[0] = 0.5;

    // Tridiagonal matrix algorithm
    for (int i = 1; i < size - 1; i++) {
        modified_matrix_term[i] = 1.0 / (4.0 - modified_matrix_term[i - 1]);
    }

    for (int i = 0; i < size; i++) {
        if (i == 0) {
            modified_result_term[i] = { result_term[i].x * 0.5
                                      , result_term[i].y * 0.5 };
        }
        else if (i == size - 1) {
            modified_result_term[i] = { (result_term[i].x - 2.0 * modified_result_term[i - 1].x) / (7.0 - 2.0 * modified_matrix_term[i - 1])
                                      , (result_term[i].y - 2.0 * modified_result_term[i - 1].y) / (7.0 - 2.0 * modified_matrix_term[i - 1]) };
        }
        else {
            modified_result_term[i] = { (result_term[i].x - modified_result_term[i - 1].x) / (4.0 - modified_matrix_term[i - 1])
                                      , (result_term[i].y - modified_result_term[i - 1].y) / (4.0 - modified_matrix_term[i - 1]) };
        }
    }

    for (int i = size - 1; i >= 0; i--) {
        if (i == size - 1) {
            double a_x = modified_result_term[i].x;
            double a_y = modified_result_term[i].y;
            
            coefficients1[i] = { a_x, a_y };
            coefficients2[i] = { (a_x + points[i + 1].x) * 0.5
                               , (a_y + points[i + 1].y) * 0.5 };
        } else {
            double a_x = modified_result_term[i].x - coefficients1[i + 1].x * modified_matrix_term[i];
            double a_y = modified_result_term[i].y - coefficients1[i + 1].y * modified_matrix_term[i];

            coefficients1[i] = { a_x, a_y };
            coefficients2[i] = { 2.0 * points[i + 1].x - coefficients1[i + 1].x
                               , 2.0 * points[i + 1].y - coefficients1[i + 1].y };
        }
    }
}

double bezier_curve_equiation(double start, double end, double coef1, double coef2, double t) {
    return pow(1 - t, 3) * start + 3 * t * pow(1 - t, 2) * coef1 + 3 * t * t * (1 - t) * coef2 + t * t * t * end;
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double x_cursor, y_cursor;

        glfwGetCursorPos(window, &x_cursor, &y_cursor);
        x_cursor = clamp(x_cursor, 0.0, (double)width);
        y_cursor = clamp(height - y_cursor, 0.0, (double)height);

        if (action == GLFW_PRESS) {
            for (vector<Point>::iterator iterator = points.begin(); iterator != points.end(); ++iterator) {
                double xGap = abs((*iterator).x - x_cursor);
                double yGap = abs((*iterator).y - y_cursor);
                
                if (xGap < 8 && yGap < 8) {
                    draggingPoint = &(*iterator);
                    break;
                }
            }
        } else if (action == GLFW_RELEASE) {
            draggingPoint = NULL;
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

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(width, height, "Bezier curve simulation", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_click_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

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

        glLineWidth(1.0);
        glBegin(GL_LINE_STRIP);
        
        fill_coefficients(points);
        
        for (int i = 0; i < points.size() - 1; i++) {
            for (int j = 0; j < interpolationCount; j++) {
                Point p1 = points[i];
                Point p2 = points[i + 1];
                double progression = (double)j / (interpolationCount - 1);
                double x = bezier_curve_equiation(p1.x, p2.x, coefficients1[i].x, coefficients2[i].x, progression);
                double y = bezier_curve_equiation(p1.y, p2.y, coefficients1[i].y, coefficients2[i].y, progression);
                double xInScreen = (x - widthHalf) / widthHalf;
                double yInScreen = (y - heightHalf) / heightHalf;
                glVertex2d(xInScreen, yInScreen);
            }
        }

        glEnd();
        glFinish();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        // Relocate the current dragging point
        if (draggingPoint != NULL) {
            double x_cursor, y_cursor;

            glfwGetCursorPos(window, &x_cursor, &y_cursor);
            x_cursor = clamp(x_cursor, 0.0, (double)width);
            y_cursor = clamp(height - y_cursor, 0.0, (double)height);

            draggingPoint->x = x_cursor;
            draggingPoint->y = y_cursor;
        }
    }

    glfwTerminate();

    return 0;
}