SOURCES	+= main.cpp helpwindow.cpp topicchooserimpl.cpp docuparser.cpp \
	   helpdialogimpl.cpp settingsdialogimpl.cpp index.cpp
HEADERS	+= helpwindow.h topicchooserimpl.h docuparser.h helpdialogimpl.h \
	   settingsdialogimpl.h index.h
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
IMAGES	= images/icon.png \
	  images/editcopy.png \
	  images/designer.xpm \
	  images/find.png \
	  images/home.png \
	  images/linguist.xpm \
	  images/next.png \
	  images/previous.png \
	  images/print.png \
	  images/qt.xpm \
	  images/whatsthis.xpm \
	  images/zoomin.xpm \
	  images/zoomout.xpm \
	  images/book.xpm

TRANSLATIONS	= assistant_de.ts \
		  assistant_fr.ts

TEMPLATE	= app
CONFIG	+= qt warn_off release
DBFILE	= assistant.db
LANGUAGE	= C++
unix:!zlib:LIBS	+= -lz
