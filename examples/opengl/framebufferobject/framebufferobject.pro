######################################################################
# Automatically generated by qmake (2.01a) Fri May 12 17:15:46 2006
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

QT += opengl svg

# Input
HEADERS += glwidget.h
SOURCES += glwidget.cpp main.cpp
RESOURCES += framebufferobject.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl/framebufferobject
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro *.png *.svg
sources.path = $$[QT_INSTALL_EXAMPLES]/opengl/framebufferobject
INSTALLS += target sources

