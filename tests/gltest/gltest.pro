REQUIRES        = opengl
TEMPLATE	= app
CONFIG		+= qt opengl warn_on debug
HEADERS		= application.h \ 
		  glinfo.h
SOURCES		= application.cpp \
		  main.cpp 
unix:SOURCES	+= glinfo_x11.cpp
win32:SOURCES   += glinfo_win.cpp
mac:SOURCES	+= glinfo_mac.cpp
TARGET		= gltest
DEPENDPATH	= ../include
