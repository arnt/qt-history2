TEMPLATE	= app
CONFIG		+= qt warn_on
TARGET		= qtconfig
SOURCES		= colorbutton.cpp main.cpp previewframe.cpp previewwidgetimpl.cpp \
		  mainwindowimpl.cpp paletteeditoradvancedimpl.cpp
HEADERS		= colorbutton.h previewframe.h previewwidgetimpl.h mainwindowimpl.h \
		  paletteeditoradvancedimpl.h
INTERFACES	= mainwindow.ui paletteeditoradvanced.ui previewwidget.ui 
IMAGEFILE	= images.cpp
PROJECTNAME	= Qt Configuration
LANGUAGE	= C++
{SOURCES+=images.cpp}
