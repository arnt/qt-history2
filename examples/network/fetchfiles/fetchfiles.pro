REQUIRES        = network large-config
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= fetchfiles.h \
                  fetchwidget.h

SOURCES		= main.cpp \
		  fetchfiles.cpp \
		  fetchwidget.cpp	

INTERFACES	= fetchwidgetbase.ui

TARGET		= testfetch
