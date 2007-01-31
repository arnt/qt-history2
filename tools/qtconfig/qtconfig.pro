TEMPLATE = app
CONFIG += qt warn_on
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
LANGUAGE = C++
RESOURCES = qtconfig.qrc

PROJECTNAME = Qt Configuration
TARGET = qtconfig
DESTDIR = ../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
INCLUDEPATH += .
DBFILE = qtconfig.db

HEADERS += colorbutton.h \
           window.h \
           preview.h \
           styledbutton.h \
           fontspage.h \
           printerpage.h \
           interfacepage.h \
           listwidget.h \
           appearancepage.h \
           undocommand.h
SOURCES += colorbutton.cpp \
           main.cpp \
           window.cpp \
           preview.cpp \
           styledbutton.cpp \
           fontspage.cpp \
           printerpage.cpp \
           interfacepage.cpp \
           listwidget.cpp \
           appearancepage.cpp \
           undocommand.cpp
FORMS += window.ui \
         preview.ui \
         fontspage.ui \
         printerpage.ui \
         interfacepage.ui \
         appearancepage.ui

