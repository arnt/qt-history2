TEMPLATE	= lib
CONFIG+= qt warn_on debug plugin

HEADERS		= ../../../../src/styles/qwindowsstyle.h \
		  ../../../../src/styles/qaquastyle.h

SOURCES		= main.cpp \
		  ../../../../src/styles/qwindowsstyle.cpp \
		  ../../../../src/styles/qaquastyle.cpp

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

TARGET		= qaquastyle
DESTDIR		= ../../../styles

target.path=$$plugins.path/styles
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/styles
INSTALLS += target
