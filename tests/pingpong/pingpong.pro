TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= pingpongfrontend.h \
		  dialogs.h \
		  cursors.h \
		  widgets.h
SOURCES		= pingpongfrontend.cpp \
		  dialogs.cpp \
		  main.cpp \
		  cursors.cpp \
		  widgets.cpp
TARGET		= pingpong
unix:LIBS      += -lpthread
