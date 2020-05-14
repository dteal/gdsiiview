#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>  // subclass
#include <QMenuBar>     // add menu items
#include <QKeySequence> // menu keyboard shortcuts
#include <QMessageBox>  // help->about dialog
#include "canvas.h"     // 3D view

class Window : public QMainWindow {
    Q_OBJECT
public:
    Canvas* canvas;
    Window();
    ~Window();
    void about();
};

#endif // WINDOW_H
