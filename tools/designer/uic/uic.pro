TEMPLATE	= app
CONFIG		+= qt console warn_on release
HEADERS	= uic.h \
		  ../shared/widgetdatabase.h \
		  ../shared/domtool.h \
		  ../integration/kdevelop/kdewidgets.h \
		  ../shared/parser.h \
		  ../interfaces/widgetinterface.h

SOURCES	= uic.cpp  \
		  ../shared/widgetdatabase.cpp  \
		  ../shared/domtool.cpp \
		  ../shared/parser.cpp \
		  ../integration/kdevelop/kdewidgets.cpp

TARGET		= uic
INCLUDEPATH	+= ../shared ../util ../../../src/3rdparty/zlib/
!zlib:unix:LIBS      += -lz

unix:LIBS	+= -lqutil -L../lib
win32:LIBS	+= $(QTDIR)/lib/qutil.lib
DEFINES 	+= UIC
DESTDIR		= ../../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
