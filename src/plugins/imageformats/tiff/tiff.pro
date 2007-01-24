TARGET  = qtiff
include(../../qpluginbase.pri)

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, no-tiff)"

SOURCES += main.cpp \
           qtiffhandler.cpp
HEADERS += qtiffhandler.h

unix:LIBS += -ltiff
win32:LIBS += libtiff.lib

QTDIR_build:DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats
target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
