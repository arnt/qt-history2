TEMPLATE    	= lib
CONFIG -= dll 
CONFIG      	+= qt x11 release staticlib
SOURCES		= qnp.cpp
unix:HEADERS += qnp.h
win32:HEADERS	= ../../../include/qnp.h
win32 {
	win32-g++ {
		LIBS	+= $$QT_BUILD_TREE\lib\libqtmain.a
	} else :LIBS	+= $$QT_BUILD_TREE\lib\qtmain.lib
}
MOC_DIR		= .
TARGET		= qnp
DESTINCDIR	= ../../../include
DESTDIR		= ../../../lib
VERSION		= 0.4
