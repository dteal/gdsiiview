#include "window.h"

Window::Window(){
    canvas = new Canvas();
    setCentralWidget(canvas);

    menubar = new QMenuBar(0);
    QMenu* file_menu = menubar->addMenu("&File");
    file_menu->addAction("&Open...", [this]{canvas->file_open();}, QKeySequence(Qt::CTRL + Qt::Key_O));
    file_menu->addAction("&Export Image...", [this]{canvas->file_save();}, QKeySequence(Qt::CTRL + Qt::Key_S));
    //file_menu->addAction("&Export STL Files..."); // TODO: implement this
    file_menu->addAction("&Exit", [this]{close();}, QKeySequence(Qt::CTRL + Qt::Key_Q));
    QMenu* view_menu = menubar->addMenu("&View");
    view_menu->addAction("&Fit", [this]{canvas->view_fit();}, QKeySequence(Qt::CTRL + Qt::Key_F));
    view_menu->addAction("&Center Model Origin", [this]{canvas->center_model_origin();});
    view_menu->addAction("&Show/Hide &Axes", [this]{canvas->toggle_axes();});
    view_menu->addAction("&Front (+X)", [this]{canvas->view_orient(0.0f, 90.0f);});
    view_menu->addAction("B&ack (-X)", [this]{canvas->view_orient(180.0f, 90.0f);});
    view_menu->addAction("&Right (+Y)", [this]{canvas->view_orient(90.0f, 90.0f);});
    view_menu->addAction("&Left (-Y)", [this]{canvas->view_orient(270.0f, 90.0f);});
    view_menu->addAction("&Top (+Z)", [this]{canvas->view_orient(0.0f, 0.0f);});
    view_menu->addAction("&Bottom (-Z)", [this]{canvas->view_orient(0.0f, 180.0f);});
    view_menu->addAction("&Isometric", [this]{canvas->view_orient(45.0f, 54.73561f);});
    setMenuBar(menubar);
}

Window::~Window(){
    delete menubar;
    delete canvas;
}
