DESTDIR       = ../../../bin
HEADERS       = displayshape.h \
                displaywidget.h \
                launcher.h
RESOURCES     = launcher.qrc
SOURCES       = displayshape.cpp \
                displaywidget.cpp \
                launcher.cpp \
                main.cpp
QT           += xml
CONFIG       += assistant

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/launcher
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS launcher.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/tools/launcher
INSTALLS += target sources
