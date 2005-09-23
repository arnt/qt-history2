TARGET        = qsvgview
HEADERS       = qsvgview.h
SOURCES       = qsvgview.cpp main.cpp
QT           += svg opengl

# install
target.path = $$[QT_INSTALL_EXAMPLES]/painting/svg
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS svg.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/painting/svg
INSTALLS += target sources
