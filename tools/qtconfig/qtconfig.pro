SOURCES	+= colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp mainwindow.cpp paletteeditoradvanced.cpp 
HEADERS	+= colorbutton.h previewframe.h previewwidget.h mainwindow.h paletteeditoradvanced.h 

TARGET		= qtconfig
DESTDIR		= ../../bin

PROJECTNAME	= Qt Configuration

target.path=$$bins.path
INSTALLS	+= target

FORMS	= mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui 
IMAGES	= images/appicon.png
TEMPLATE	=app
CONFIG	+= qt warn_on
INCLUDEPATH	+= .
DBFILE	= qtconfig.db
LANGUAGE	= C++
