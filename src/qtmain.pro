TEMPLATE	= lib
CONFIG		= qt staticlib warn_on release
win32:SOURCES	= kernel/qtmain_win.cpp
DEFINES		= QT_DLL
TARGET		= qtmain
VERSION		= 1.41
DESTDIR		= ../lib
