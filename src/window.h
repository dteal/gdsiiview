#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QKeySequence>
#include "canvas.h"

class Window : public QMainWindow {
    Q_OBJECT
public:
    QMenuBar* menubar;
    Canvas* canvas;
    Window();
    ~Window();
};

#endif // WINDOW_H
