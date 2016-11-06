#-------------------------------------------------
#
# Project created by QtCreator 2016-10-30T13:34:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = First
CONFIG   += console gl_debug
CONFIG   -= app_bundle
INCLUDEPATH += $$PWD/OpenGL

TEMPLATE = app


SOURCES += main.cpp \
    glwindow.cpp \
    mainwindow.cpp \
    qtriangle.cpp \
    transform3d.cpp \
    input.cpp \
    camera3d.cpp \
    OpenGL/openglerror.cpp

HEADERS  += \ # mainwindow.h #\
    glwindow.h \
    mainwindow.h \
    vertex.h \
    transform3d.h \
    input.h \
    camera3d.h \
    openglerror.h \
    macros.h \
    OpenGL/openglbuffer.h \
    OpenGL/openglcommon.h \
    OpenGL/openglerror.h \
    OpenGL/openglshaderprogram.h \
    OpenGL/openglvertexarrayobject.h

FORMS    += mainwindow.ui

RESOURCES += \
    first.qrc

#LIBS += -lglu
#LIBS += -L$$PWD/lib/glut32.lib
#LIBS += -L$$PWD/lib/glew32.lib
#LIBS += -L$$PWD/lib/OpenGL32.lib

DISTFILES += \
    shaders/simple.vert \
    shaders/simple.frag \
    OpenGL/OpenGLBuffer \
    OpenGL/OpenGLCommon \
    OpenGL/OpenGLError \
    OpenGL/OpenGLShaderProgram \
    OpenGL/OpenGLVertexArrayObject


CONFIG += gl_debug
CONFIG(debug,debug|release) {
  CONFIG(gl_debug) {
      DEFINES += GL_DEBUG
  }
}
