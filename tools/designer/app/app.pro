TEMPLATE 	= app
CONFIG 		-= moc

SOURCES		+= main.cpp
unix:LIBS	+= -ldesigner -L$(QTDIR)/lib
win32:LIBS	+= $(QTDIR)/lib/designerlib.lib
INCLUDEPATH	+= ../designer
TARGET		= designer
DESTDIR		= $(QTDIR)/bin
win32:RC_FILE	= designer.rc
mac:RC_FILE	= designer.icns

target.path=$$bins.path
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS        += target
