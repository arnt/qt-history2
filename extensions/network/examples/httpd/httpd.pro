TEMPLATE	= app
CONFIG		= qt warn_on release
TMAKE_CXXFLAGS  = -I../../src
unix:LIBS       = -lqnetwork
win32:LIBS      = $(QTDIR)\lib\qnetwork.lib
HEADERS		= 
SOURCES		= httpd.cpp
TARGET		= httpd
