TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= composer.h \
		  smtp.h
SOURCES		= composer.cpp \
		  main.cpp \
		  smtp.cpp
TARGET		= mail
unix:LIBS	= -lqnetwork
win32:LIBS	= $(QTDIR)/lib/qnetwork.lib
