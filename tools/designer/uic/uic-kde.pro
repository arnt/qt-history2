TEMPLATE	= app
CONFIG		+= qt console warn_on release
HEADERS		= uic.h ../shared/widgetdatabase.h ../shared/domtool.h ../integration/kdevelop/kdewidgets.h
SOURCES		= uic.cpp ../shared/widgetdatabase.cpp ../shared/domtool.cpp ../integration/kdevelop/kdewidgets.cpp
include( ../../../src/qt_professional.pri )
TARGET		= uic
INCLUDEPATH	+= ../shared ../../../src/3rdparty/zlib/ $(KDEDIR)/include
unix:LIBS	+= -L$(KDEDIR)/lib -lkdecore -lkdeui -lDCOP
DEFINES 	+= UIC HAVE_KDE
DESTDIR		= ../../../bin
