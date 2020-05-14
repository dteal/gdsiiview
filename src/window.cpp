#include "window.h"

Window::Window(){
    canvas = new Canvas();
    setCentralWidget(canvas);

    QMenu* file_menu = menuBar()->addMenu("&File");
    file_menu->addAction("&Open...",            [this]{canvas->file_open();}, QKeySequence(Qt::CTRL + Qt::Key_O));
    file_menu->addAction("&Export Image...",    [this]{canvas->file_save();}, QKeySequence(Qt::CTRL + Qt::Key_S));
    file_menu->addAction("&Exit",               [this]{close();}, QKeySequence(Qt::CTRL + Qt::Key_Q));
    //file_menu->addAction("&Export STL Files..."); // TODO: implement this

    QMenu* view_menu = menuBar()->addMenu("&View");
    view_menu->addAction("&Fit",                [this]{canvas->view_fit();}, QKeySequence(Qt::CTRL + Qt::Key_F));
    view_menu->addAction("&Center Model Origin",[this]{canvas->center_model_origin();});
    view_menu->addAction("&Show/Hide &Axes",    [this]{canvas->toggle_axes();});
    view_menu->addAction("&Front (+X)",         [this]{canvas->view_orient(0.0f, 90.0f);});
    view_menu->addAction("&Back (-X)",          [this]{canvas->view_orient(180.0f, 90.0f);});
    view_menu->addAction("&Right (+Y)",         [this]{canvas->view_orient(90.0f, 90.0f);});
    view_menu->addAction("&Left (-Y)",          [this]{canvas->view_orient(270.0f, 90.0f);});
    view_menu->addAction("&Top (+Z)",           [this]{canvas->view_orient(0.0f, 0.0f);});
    view_menu->addAction("B&ottom (-Z)",        [this]{canvas->view_orient(0.0f, 180.0f);});
    view_menu->addAction("&Isometric",          [this]{canvas->view_orient(45.0f, 54.73561f);});

    QMenu* help_menu = menuBar()->addMenu("&Help");
    help_menu->addAction("&About gdsiiview...", [this]{about();});
}

Window::~Window(){
    delete canvas;
}

void Window::about(){
    QString about_info =
        "<h3>About gdsiiview</h3>"
        "<p>This program displays GDSII files as 3D geometry.</p>"
        "<p>Copyright © 2020 <a href=\"mailto:dteal@dteal.org\">Daniel Teal</a></p>"
        "<p>"
        "For documentation and source code, see <br>"
        "<a href=\"https://github.com/dteal/gdsiiview\">https://github.com/dteal/gdsiiview</a>."
        "</p><p>"
        "This program is licensed under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU GPL v3.0</a>."
        "</p><p>"
        "This program is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or "
        "(at your option) any later version."
        "</p><p>"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
        "GNU General Public License for more details."
        "</p><p>"
        "This program uses the following libraries:"
        "</p><p>"
        "<ul>"
        "<li><a href=\"https://www.qt.io/\">Qt 5</a></li>"
        "<li><a href=\"https://glm.g-truc.net/0.9.9/index.html\">GLM</a></li>"
        "<li><a href=\"https://www.cs.cmu.edu/~quake/triangle.html\">Jonathan Shewchuk's Triangle library</a></li>"
        "</ul></p>";
    QMessageBox::about(this, "About gdsiiview", about_info);
}
