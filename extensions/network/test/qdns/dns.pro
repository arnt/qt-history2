TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= 
SOURCES		= dns.cpp
INTERFACES	= 
TARGET		= dns
unix:LIBS       = -lqnetwork
win32:LIBS      = $(QTDIR)\lib\qnetwork.lib
