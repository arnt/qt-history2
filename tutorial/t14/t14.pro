# Project ID used by some IDEs
GUID 		= {827a63ea-1837-4310-94e3-e1473a35a359}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  gamebrd.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  gamebrd.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t14
QTDIR_build:REQUIRES=full-config
unix:LIBS += -lm
