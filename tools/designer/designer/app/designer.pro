TEMPLATE 	= app
CONFIG 		-= moc

SOURCES		+= main.cpp
unix:LIBS		+= -ldesigner -L$(QTDIR)/lib
win32:LIBS	+= $(QTDIR)/lib/designer.lib
INCLUDEPATH	+= ..
TARGET		= designer
DESTDIR		= $(QTDIR)/bin


