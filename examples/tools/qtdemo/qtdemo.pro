CONFIG        += assistant
DESTDIR       = ../../../bin
HEADERS       = displayshape.h \
                displaywidget.h \
                launcher.h
QT            += xml
RESOURCES     = qtdemo.qrc
SOURCES       = displayshape.cpp \
                displaywidget.cpp \
                launcher.cpp \
                main.cpp
TARGET        = qtdemo

# install
target.path = $$[QT_INSTALL_DATA]/examples/tools/qtdemo
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qtdemo.pro images
sources.path = $$[QT_INSTALL_DATA]/examples/tools/qtdemo
INSTALLS += target sources
