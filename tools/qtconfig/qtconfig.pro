TEMPLATE	= app
CONFIG		+= qt warn_on
TARGET		= qtconfig
SOURCES		= colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp \
		  mainwindow.cpp paletteeditoradvanced.cpp
HEADERS		= colorbutton.h previewframe.h previewwidget.h mainwindow.h \
		  paletteeditoradvanced.h
DESTDIR		= ../../bin
INCLUDEPATH	+= .
INTERFACES	= mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui 
DBFILE	= qtconfig.db
IMAGEFILE	= images.cpp
PROJECTNAME	= Qt Configuration
LANGUAGE	= C++
{SOURCES+=images.cpp}
