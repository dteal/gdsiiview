#include "window.h"

Window::Window(){
    canvas = new Canvas();
    setCentralWidget(canvas);

    menubar = new QMenuBar(0);
    QMenu* file_menu = menubar->addMenu("&File");
    file_menu->addAction("&Open...", [this]{canvas->file_open();}, QKeySequence(Qt::CTRL + Qt::Key_O));
    file_menu->addAction("&Save Image...", [this]{canvas->file_save();}, QKeySequence(Qt::CTRL + Qt::Key_S));
    file_menu->addAction("&Exit", [this]{close();}, QKeySequence(Qt::CTRL + Qt::Key_Q));
    QMenu* view_menu = menubar->addMenu("&View");
    view_menu->addAction("&Fit", [this]{canvas->view_fit();}, QKeySequence(Qt::CTRL + Qt::Key_F));
    // TODO: make these match multiview projection standards
    view_menu->addAction("&Perspective", [this]{canvas->view_perspective();}, QKeySequence(Qt::CTRL + Qt::Key_P));
    view_menu->addAction("O&rthographic", [this]{canvas->view_orthographic();}, QKeySequence(Qt::CTRL + Qt::Key_R));
    view_menu->addAction("&Front (+X)");
    view_menu->addAction("B&ack (-X)");
    view_menu->addAction("&Right (+Y)");
    view_menu->addAction("&Left (-Y)");
    view_menu->addAction("&Top (+Z)");
    view_menu->addAction("&Bottom (-Z)");
    view_menu->addAction("&Isometric");
    setMenuBar(menubar);
}

Window::~Window(){
    delete menubar;
    delete canvas;
}
