TEMPLATE	= lib
CONFIG		= qt staticlib release
HEADERS		= qimageio.h \
		  qjpegio.h
SOURCES		= qimageio.cpp \
		  qjpegio.cpp
unix:LIBS	= -ljpeg
win32:LIBS	= jpeg.lib
TARGET		= qimgio
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.1
