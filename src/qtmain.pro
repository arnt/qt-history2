TEMPLATE	= lib
CONFIG		= qt staticlib warn_on release
win32:SOURCES	= kernel/qtinit_win.cpp \
		  kernel/qtmain_win.cpp
DEFINES		= QT_DLL
TARGET		= qtmain
VERSION		= 2.00
DESTDIR		= ../lib

win32:TMAKE_CFLAGS     += -DUNICODE
win32:TMAKE_CXXFLAGS   += -DUNICODE

#win32:TMAKE_CFLAGS     += -MT
#win32:TMAKE_CXXFLAGS   += -MT
