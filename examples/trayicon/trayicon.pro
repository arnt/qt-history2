TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= trayicon.h
SOURCES		= main.cpp \
		  trayicon.cpp
win32:SOURCES  += trayicon_win.cpp
embedded:SOURCES+=trayicon_qws.cpp
INTERFACES	= 

TARGET		= trayicon
REQUIRES	= large-config
