TARGET        = qsvgview
HEADERS       = qsvgview.h
SOURCES       = qsvgview.cpp main.cpp
QT           += svg 

contains(QT_CONFIG, opengl): QT += opengl

CONFIG += console

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting/svg
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS svg.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/painting/svg
INSTALLS += target sources
