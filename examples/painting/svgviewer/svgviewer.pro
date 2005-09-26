TARGET        = svgview
HEADERS       = svgview.h
SOURCES       = svgview.cpp main.cpp
QT           += svg

contains(QT_CONFIG, opengl): QT += opengl

CONFIG += console

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting/svgview
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS svgview.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/painting/svgview
INSTALLS += target sources
