TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= centralwidget.h \
		  mainwindow.h
SOURCES		= centralwidget.cpp \
		  main.cpp \
		  mainwindow.cpp
TARGET		= qutlook

LIBS        += qaxcontainer.lib
