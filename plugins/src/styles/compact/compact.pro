TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../include/qwindowsstyle.h \
		  ../../../../include/qcompactstyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsstyle.cpp \
		  ../../../../src/styles/qcompactstyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qcompactstyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
