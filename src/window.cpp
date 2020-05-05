#include "window.h"

Window::Window(){
    QMenuBar* menubar = new QMenuBar(0);
    QMenu* file_menu = menubar->addMenu("&File");
    file_menu->addAction("&Open...");
    file_menu->addAction("&Save Image...");
    file_menu->addAction("&Exit");
    QMenu* view_menu = menubar->addMenu("&View");
    view_menu->addAction("&Fit");
    view_menu->addAction("&Perspective");
    view_menu->addAction("&Orthographic");
    view_menu->addAction("&Front (+X)");
    view_menu->addAction("B&ack (-X)");
    view_menu->addAction("&Right (+Y)");
    view_menu->addAction("&Left (-Y)");
    view_menu->addAction("&Top (+Z)");
    view_menu->addAction("&Bottom (-Z)");
    view_menu->addAction("&Isometric");
    QMenu* window_menu = menubar->addMenu("&Window");
    window_menu->addAction("Toggle Window &Transparency");
    window_menu->addAction("Toggle &Menu Bar");
    setMenuBar(menubar);

    Canvas* canvas = new Canvas();
    setCentralWidget(canvas);
}
