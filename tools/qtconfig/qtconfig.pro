TEMPLATE	= app
CONFIG		+= qt warn_on
SOURCES		= colorbutton.cpp main.cpp previewframe.cpp previewwidget.cpp \
		  mainwindow.cpp paletteeditoradvanced.cpp
HEADERS		= colorbutton.h previewframe.h previewwidget.h mainwindow.h \
		  paletteeditoradvanced.h
FORMS		= mainwindowbase.ui paletteeditoradvancedbase.ui previewwidgetbase.ui 

DBFILE		= qtconfig.db
IMAGEFILE	= images.cpp
{SOURCES+=images.cpp}

TARGET		= qtconfig
INCLUDEPATH	+= .
DESTDIR		= ../../bin

PROJECTNAME	= Qt Configuration

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS	+= target
