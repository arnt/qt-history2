SOURCES	+= main.cpp \
	helpwindow.cpp \
	topicchooserimpl.cpp \
	docuparser.cpp \
	helpdialogimpl.cpp \
	settingsdialogimpl.cpp \
	index.cpp
HEADERS	+= helpwindow.h \
	topicchooserimpl.h \
	docuparser.h \
	helpdialogimpl.h \
	settingsdialogimpl.h \
	index.h
TARGET	= assistant

#DEFINES +=  QT_PALMTOPCENTER_DOCS
include( ../../src/qt_professional.pri )
DESTDIR	= ../../bin

win32:RC_FILE = assistant.rc
mac:RC_FILE = assistant.icns


target.path=$$bins.path
INSTALLS += target

PROJECTNAME	= Assistant

TRANSLATIONS	= assistant_de.ts \
		  assistant_fr.ts

unix:!zlib:LIBS	+= -lz
DEFINES	+= QT_INTERNAL_NETWORK
FORMS	= mainwindow.ui \
	topicchooser.ui \
	finddialog.ui \
	helpdialog.ui \
	settingsdialog.ui
IMAGES	= images/appicon.png \
	images/editcopy.png \
	images/find.png \
	images/home.png \
	images/next.png \
	images/previous.png \
	images/print.png \
	images/qt.xpm \
	images/whatsthis.xpm \
	images/zoomin.xpm \
	images/zoomout.xpm \
	images/book.png \
	images/designer.png \
	images/assistant.png \
	images/linguist.png
TEMPLATE	=app
CONFIG	+= qt warn_off release
DEFINES	+= QT_INTERNAL_NETWORK
LANGUAGE	= C++
