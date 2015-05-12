TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
    Cube.cpp \
    Plane.cpp \
    TexturedCube.cpp \
    View.cpp

RESOURCES += qml.qrc \
    image.qrc \
    shader.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    Cube.h \
    Plane.h \
    TexturedCube.h \
    View.h
