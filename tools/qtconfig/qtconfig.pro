SOURCES	+= colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp mainwindow.cpp paletteeditoradvanced.cpp 
HEADERS	+= colorbutton.h previewframe.h previewwidget.h mainwindow.h paletteeditoradvanced.h 

IMAGEFILE	= images.cpp

TARGET		= qtconfig
DESTDIR		= ../../bin

PROJECTNAME	= Qt Configuration

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS	+= target
FORMS	= mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui 
IMAGES	= images/filesave 
TEMPLATE	=app
CONFIG	+= qt warn_on
INCLUDEPATH	+= .
DBFILE	= qtconfig.db
LANGUAGE	= C++
