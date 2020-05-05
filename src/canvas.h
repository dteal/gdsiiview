#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
//#include "part.h"
//#include "mesh.h"

extern "C" {
    #define ANSI_DECLARATORS
    typedef double REAL;
    typedef void VOID; // required on linux (maybe not Windows?)
    #include "triangle.h"
}

class Canvas : public QOpenGLWidget, public QOpenGLFunctions {
public:
    glm::vec2 cursor_position = glm::vec2(0.0f, 0.0f);
    glm::vec2 screen_size = glm::vec2(0.0f, 0.0f);
    /*
    glm::vec3 camera_position = glm::vec3(-1000.0f, 0.0f, 0.0f);
    glm::vec3 camera_direction = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 0.0f, 1.0f);
    */
    bool orbiting = false;
    float theta = 45.0f; // camera angle, degrees
    float phi = M_PI/2.0f; // camera angle, degrees
    float zoom = 0.0005f;
    //std::string config_filepath;
    //std::time_t config_write_time;

    //std::vector<std::shared_ptr<Part>>parts;

    Canvas();
    ~Canvas();

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    bool eventFilter(QObject*, QEvent* event);
    void cursor_scroll_callback();

    bool parse_configuration(std::string filepath);
};

#endif // CANVAS_H
