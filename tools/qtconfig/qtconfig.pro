TEMPLATE = app
CONFIG        += qt warn_on uic3
LANGUAGE = C++
QT += qt3support
SOURCES        += colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp mainwindow.cpp paletteeditoradvanced.cpp
HEADERS        += colorbutton.h previewframe.h previewwidget.h mainwindow.h paletteeditoradvanced.h
FORMS        = mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui
IMAGES        = images/appicon.png

PROJECTNAME        = Qt Configuration
TARGET                = qtconfig
DESTDIR                = ../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
INCLUDEPATH        += .
DBFILE                 = qtconfig.db
