SOURCES	+= main.cpp helpwindow.cpp topicchooserimpl.cpp helpdialogimpl.cpp settingsdialogimpl.cpp assistant.cpp 
HEADERS	+= helpwindow.h topicchooserimpl.h helpdialogimpl.h settingsdialogimpl.h assistant.h 
TARGET	= assistant

DEFINES += QT_INTERNAL_NETWORK
include( ../../src/qt_professional.pri )
DESTDIR	= ../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
PROJECTNAME	= Assistant
FORMS	= mainwindow.ui topicchooser.ui finddialog.ui helpdialog.ui settingsdialog.ui 

TRANSLATIONS	= assistant_de.ts \
		  assistant_fr.ts

TEMPLATE	=app
CONFIG	+= qt warn_off release
DBFILE	= assistant.db
LANGUAGE	= C++
