TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= commands.h \
		  infotext.h \
		  launcher.h \
		  quickbutton.h \
		  sourceviewer.h
SOURCES		+= launcher.cpp \
		   main.cpp \
		   quickbutton.cpp \
		   sourceviewer.cpp
INTERFACES	= 
TARGET		= launcher
