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
	
FORMS	= mainwindow.ui \
	topicchooser.ui \
	finddialog.ui \
	helpdialog.ui \
	settingsdialog.ui \
	tabbedbrowser.ui
IMAGES	= images/editcopy.png \
	images/find.png \
	images/home.png \
	images/next.png \
	images/previous.png \
	images/print.png \
	images/whatsthis.xpm \
	images/book.png \
	images/designer.png \
	images/assistant.png \
	images/linguist.png \
	images/qt.png \
	images/zoomin.png \
	images/zoomout.png \
	images/splash.png \
	images/appicon.png
	
TEMPLATE	=app
CONFIG	+= qt
DEFINES	+= QT_INTERNAL_NETWORK QT_INTERNAL_XML
LANGUAGE	= C++
