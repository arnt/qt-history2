TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS	= ftpmainwindow.h \
		ftpview.h
SOURCES	= ftpmainwindow.cpp \
		ftpview.cpp \
		  main.cpp
TARGET		= ftpclient
unix:LIBS	= -lqnetwork
win32:LIBS	= $(QTDIR)/qnetwork.lib

