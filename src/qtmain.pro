# Additional Qt project file for qtmain lib on Windows
TEMPLATE	= lib
TARGET		= qtmain
VERSION		= 3.0.1
DESTDIR		= $$QMAKE_LIBDIR_QT

CONFIG		+= qt staticlib warn_on release
CONFIG		-= dll

win32 {
	SOURCES		= kernel/qtmain_win.cpp
	CONFIG		+= png zlib
	CONFIG		-= jpeg
	DEFINES		+= UNICODE
	INCLUDEPATH	+= tmp
	MOC_DIR		= tmp
	OBJECTS_DIR	= tmp
}
win32-borland:INCLUDEPATH += kernel
