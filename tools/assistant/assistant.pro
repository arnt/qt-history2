SOURCES	+= main.cpp helpwindow.cpp topicchooserimpl.cpp docuparser.cpp \
	   helpdialogimpl.cpp settingsdialogimpl.cpp assistant.cpp index.cpp
HEADERS	+= helpwindow.h topicchooserimpl.h docuparser.h helpdialogimpl.h \
	   settingsdialogimpl.h assistant.h index.h
TARGET	= assistant

DEFINES += QT_INTERNAL_NETWORK
DEFINES += QT_INTERNAL_XML
#DEFINES +=  QT_PALMTOPCENTER_DOCS
include( ../../src/qt_professional.pri )
DESTDIR	= ../../bin

win32:RC_FILE = assistant.rc

target.path=$$bins.path
INSTALLS += target

PROJECTNAME	= Assistant
FORMS	= mainwindow.ui topicchooser.ui finddialog.ui helpdialog.ui settingsdialog.ui 
IMAGES	= images/icon.png images/copy.xpm images/designer.xpm images/find.xpm images/home.xpm images/linguist.xpm images/next.xpm images/previous.xpm images/print.xpm images/qt.xpm images/whatsthis.xpm images/zoomin.xpm images/zoomout.xpm

TRANSLATIONS	= assistant_de.ts \
		  assistant_fr.ts

TEMPLATE	= app
CONFIG	+= qt warn_off release
DBFILE	= assistant.db
LANGUAGE	= C++
unix:!zlib:LIBS	+= -lz
