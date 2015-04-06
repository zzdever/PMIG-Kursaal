#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T10:09:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
qtHaveModule(printsupport): QT += printsupport
qtHaveModule(opengl): QT += opengl

TARGET = PMIG-Kursaal
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    opencvprocess.cpp \
    scribblearea.cpp \
    panels.cpp \
    shared/hoverpoints.cpp \
    toolbox.cpp \
    2DEngine/Shapes/P2DPolygonShape.cpp \
    2DEngine/General/P2DMath.cpp \
    2DEngine/General/P2DParams.cpp \
    playground.cpp

HEADERS  += mainwindow.h \
    scribblearea.h \
    panels.h \
    shared/hoverpoints.h \
    toolbox.h \
    2DEngine/Shapes/P2DBaseShape.h \
    2DEngine/Shapes/P2DPolygonShape.h \
    2DEngine/General/P2DMath.h \
    2DEngine/General/P2DParams.h \
    playground.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc




INCLUDEPATH += /usr/local/include/opencv/
INCLUDEPATH += /usr/local/include/


LIBS += /usr/local/lib/libopencv_core.dylib\
        /usr/local/lib/libopencv_highgui.dylib\
        /usr/local/lib/libopencv_imgproc.dylib
