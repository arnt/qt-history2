TEMPLATE 	= app
CONFIG 		-= moc

SOURCES		+= main.cpp
unix:LIBS	+= -ldesigner -L$(QTDIR)/lib
win32:LIBS	+= $(QTDIR)/lib/designerlib.lib
INCLUDEPATH	+= ..
TARGET		= designer
DESTDIR		= $(QTDIR)/bin
win32:RC_FILE	= designer.rc
mac:RC_FILE	= designer.icns
