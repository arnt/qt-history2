TEMPLATE 	= app
DESTDIR		= $$QT_BUILD_TREE/bin
TARGET		= designer
CONFIG 		-= moc

SOURCES		+= main.cpp
INCLUDEPATH	+= ../designer
LIBS		+= -L$$QT_BUILD_TREE/lib -ldesigner -lqui -lqassistantclient 
win32:RC_FILE	= designer.rc


target.path=$$bins.path
INSTALLS        += target
