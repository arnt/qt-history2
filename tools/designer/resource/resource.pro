TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
win32:CONFIG	-= dll
SOURCES		= qwidgetfactory.cpp \
		  ../shared/domtool.cpp \
		  ../integration/kdevelop/kdewidgets.cpp

HEADERS		= qwidgetfactory.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h

sql:SOURCES += 		  ../designer/database.cpp
sql:HEADERS +=		  ../designer/database.h

TARGET		= qresource
INCLUDEPATH	= ../shared ../util ../../../src/3rdparty/zlib/
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0
DEFINES		+= RESOURCE

unix {
	target.path=$$QT_INSTALL_LIBPATH
	isEmpty(target.path):target.path=$$QT_PREFIX/lib
	INSTALLS        += target
}
