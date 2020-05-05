#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDebug>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

extern "C" {
    #define ANSI_DECLARATORS
    typedef double REAL;
    typedef void VOID; // required on linux (maybe not Windows?)
    #include "triangle.h"
}

class Canvas : public QOpenGLWidget, public QOpenGLFunctions {
public:
    Canvas();
    ~Canvas();

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
};

#endif // CANVAS_H
