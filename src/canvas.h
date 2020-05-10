#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QFileSystemWatcher>
#include <QFileDialog> // open/save dialogs
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent> // mouse for orbit/pan
#include <QWheelEvent> // mouse scrolling for zoom
#include <QString>
#include <QPoint>
#include <memory>
#include <iostream>
#include <sstream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "part.h"
#include "mesh.h"
#include "axes.h"

// This class loads and renders a 3D view of a single *.gdsiiview file;
// it holds a large portion of the entire application code.
class Canvas : public QOpenGLWidget, protected QOpenGLFunctions {
public:

    // The cursor position and screen size are tracked to manipulate the camera.
    glm::vec2 cursor_position = glm::vec2(0.0f, 0.0f); // location of mouse (pixels, y up)
    glm::vec2 screen_size = glm::vec2(0.0f, 0.0f); // size of rendered image (pixels)

    // The camera always looks from infinity at (camera_theta, camera_phi)
    // toward a view origin at (camera_position). This origin is coincident
    // with the model origin when the model is first loaded, but can be
    // translated along the plane currently perpendicular to the camera by
    // panning. The zoom gives the length of a line on that perpendicular
    // plane centered at the view origin.
    glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 0.0f); // position of view origin w.r.t. model origin (model units)
    float camera_theta = 45.0f; // camera horizontal angle (degrees down from z axis)
    float camera_phi = M_PI/2.0f; // camera vertical angle (degrees CCW from x axis on xy plane)
    float camera_zoom = 0.0005f; // model display size (model units per pixel)
    bool camera_orbiting = false; // whether mouse is dragging to rotate view
    bool camera_panning = false; // whether mouse is dragging to shift view

    // Background color displayed behind the loaded model.
    glm::vec3 background_color = glm::vec3(0.1f, 0.1f, 0.1f);
    Axes* axes;

    // One *.gdsiiview file can be loaded at a time; its filepath is stored
    // in (filepath). When (watcher) detects this file is changed, it is
    // automatically reloaded---this way, this program can serve as a
    // low-latency visualization aid while editing the file in a (usually 2D)
    // GDSII editing program. (parts) stores each reference to GDSII files
    // in the *.gdsiiview file; these files are also watched, separately.
    QString filepath = "";
    QFileSystemWatcher* watcher;
    std::vector<std::shared_ptr<Part>>parts;

    Canvas();
    ~Canvas();
    void initializeGL(); // OpenGL is first active in this function
    void resizeGL(int width, int height); // called whenever window is resized
    void paintGL(); // main drawing function; called whenever window is updated
    bool eventFilter(QObject*, QEvent* event); // handle mouse, keyboard
    bool initialize_from_file(QString filepath); // load *.gdsiiview file

public slots:
    void update_file(QString filepath); // discard current and load new file
    void file_open(); // choose and open file with GUI dialog
    void file_save(); // choose and save rendered image with GUI dialog
    void view_fit(); // adjust zoom to fit model in screen (camera view)
    void view_perspective(); // change to perspective camera view
    void view_orthographic(); // change to orthographic camera view
    void view_orient(glm::vec3 direction, glm::vec3 up); // TODO: update this
};

#endif // CANVAS_H
