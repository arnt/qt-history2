TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= pingpongapp.h \
		  dialogs.h \
		  cursors.h
SOURCES		= pingpongapp.cpp \
		  dialogs.cpp \
		  main.cpp \
		  cursors.cpp
TARGET		= pingpong
unix:LIBS      += -lpthread
