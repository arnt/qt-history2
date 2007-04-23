CONFIG += assistant x11inc
CONFIG += release
TARGET = qtdemo
DESTDIR = $$QT_BUILD_TREE/bin
OBJECTS_DIR = .obj
MOC_DIR = .moc
INSTALLS += target sources
QT += xml network
contains(QT_CONFIG, opengl): QT += opengl

RESOURCES = qtdemo.qrc
HEADERS = src/mainwindow.h \
    src/demoscene.h \
    src/demoitem.h \
    src/score.h \
    src/demoitemanimation.h \
    src/itemcircleanimation.h \
    src/demotextitem.h \
    src/headingitem.h \
    src/dockitem.h \
    src/scanitem.h \
    src/letteritem.h \
    src/examplecontent.h \
    src/menucontent.h \
    src/guide.h \
    src/guideline.h \
    src/guidecircle.h \
    src/menumanager.h \
    src/colors.h \
    src/textbutton.h \
    src/imageitem.h
SOURCES = src/main.cpp \
    src/demoscene.cpp \
    src/mainwindow.cpp \
    src/demoitem.cpp \
    src/score.cpp \
    src/demoitemanimation.cpp \
    src/itemcircleanimation.cpp \
    src/demotextitem.cpp \
    src/headingitem.cpp \
    src/dockitem.cpp \
    src/scanitem.cpp \
    src/letteritem.cpp \
    src/examplecontent.cpp \
    src/menucontent.cpp \
    src/guide.cpp \
    src/guideline.cpp \
    src/guidecircle.cpp \
    src/menumanager.cpp \
    src/colors.cpp \
    src/textbutton.cpp \
    src/imageitem.cpp

win32:RC_FILE = qtdemo.rc
mac:ICON = qtdemo.icns

# install
target.path = $$[QT_INSTALL_BINS]
sources.files = $$SOURCES $$HEADERS $$RESOURCES qtdemo.pro images xml *.ico *.icns *.rc
sources.path = $$[QT_INSTALL_DEMOS]/qtdemo
