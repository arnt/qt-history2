TEMPLATE	= app
CONFIG += qt warn_on debug
TARGET	= assistant
SOURCES	= main.cpp helpwindow.cpp topicchooserimpl.cpp helpdialogimpl.cpp assistant.cpp
HEADERS	= helpwindow.h topicchooserimpl.h helpdialogimpl.h assistant.h
include( ../../src/qt_professional.pri )
DESTDIR	= ../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
INTERFACES	= mainwindow.ui topicchooser.ui finddialog.ui helpdialog.ui 
DBFILE	= assistant.db
PROJECTNAME	= Assistant
LANGUAGE	= C++
