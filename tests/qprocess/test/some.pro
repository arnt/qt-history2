TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= ../qprocess.h \
		  some.h
win32:SOURCES	= ../qprocess_win.cpp
unix:SOURCES	= ../qprocess_unix.cpp
SOURCES		+= ../qprocess.cpp \
		   main.cpp \
		   some.cpp
INTERFACES	= 
TARGET		= some
