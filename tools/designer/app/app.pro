TEMPLATE 	= app
CONFIG 		-= moc

SOURCES		+= main.cpp
unix:LIBS	+= -ldesigner -L$$QT_BUILD_TREE/lib
win32:LIBS	+= $$QT_BUILD_TREE/lib/designerlib.lib
INCLUDEPATH	+= ../designer
TARGET		= designer
DESTDIR		= $$QT_BUILD_TREE/bin
win32:RC_FILE	= designer.rc
mac:RC_FILE	= designer.icns

target.path=$$bins.path

INSTALLS        += target
