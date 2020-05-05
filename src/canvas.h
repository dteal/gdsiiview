#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QFileSystemWatcher>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>
#include <memory>
#include <iostream>
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

    glm::vec3 background_color = glm::vec3(0.1f, 0.1f, 0.1f);
    //std::vector<std::shared_ptr<Part>>parts;

    QString filepath = "";
    QFileSystemWatcher* watcher;

    Canvas();
    ~Canvas();

    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

    bool eventFilter(QObject*, QEvent* event);

    bool initialize_from_file(QString filepath);

public slots:
    void update_file(QString filepath);
    void file_open();
    void file_save();
    void view_fit();
    void view_perspective();
    void view_orthographic();
    void view_orient(glm::vec3 direction, glm::vec3 up);
};

#endif // CANVAS_H
