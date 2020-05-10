QT += core gui opengl
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    src/thirdparty/glm \
    src/thirdparty/triangle

SOURCES += \
    src/main.cpp \
    src/window.cpp \
    src/canvas.cpp \
    src/thirdparty/triangle/triangle.c

HEADERS += \
    src/window.h \
    src/canvas.h \
    src/axes.h \
    src/parts/part.h \
    src/parts/gdsii.h \
    src/parts/gdsiimesh.h \
    src/parts/image.h \
    src/thirdparty/triangle/triangle.h

# For compilation of Triangle library:
QMAKE_CFLAGS += -O1
QMAKE_CFLAGS += -DNO_TIMER
QMAKE_CFLAGS += -DTRILIBRARY
# These flags were recommended in the original Triangle makefile for exact
# floating computation, but I'm not sure if they will work correctly.
win32{
    QMAKE_CFLAGS += -DCPU86
}
unix:!macx{
    QMAKE_CFLAGS += -DLINUX
}
# Triangle library is kinda messy
CONFIG += warn_off
