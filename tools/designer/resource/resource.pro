TEMPLATE	= lib
OBJECTS_DIR	= .
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
SOURCES		= qwidgetfactory.cpp \
		  ../shared/widgetdatabase.cpp \
		  ../shared/domtool.cpp \
		  ../integration/kdevelop/kdewidgets.cpp \
		  ../designer/config.cpp \
		  ../designer/pixmapchooser.cpp \
		  ../shared/guid.cpp
HEADERS		= qwidgetfactory.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h \
		  ../designer/config.h \
		  ../designer/pixmapchooser.h

sql:SOURCES += 		  ../designer/database.cpp 
sql:HEADERS +=		  ../designer/database.h 

TARGET		= qresource
INCLUDEPATH	= ../shared ../util ../../../src/3rdparty/zlib/
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
DEFINES		+= RESOURCE
