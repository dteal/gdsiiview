#include "window.h"

#include <QApplication>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    Window window;
    window.resize(500, 500);
    window.setWindowTitle("GDSII 3D Viewer");
    window.show();

    return app.exec();
}
