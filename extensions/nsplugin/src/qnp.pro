TEMPLATE    	= lib
CONFIG -= dll 
CONFIG      	+= qt x11 release staticlib
SOURCES		= qnp.cpp
x11:HEADERS += qnp.h qxt.h
x11:SOURCES += qxt.cpp
win32:HEADERS	= ../../../include/qnp.h
win32:LIBS	+= $$QT_BUILD_TREE\lib\qtmain.lib
MOC_DIR		= .
TARGET		= qnp
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.3
