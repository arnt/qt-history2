# Project ID used by some IDEs
GUID 		= {b9ed6045-e3f6-4619-8f9c-e20cd6a58410}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  gamebrd.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  gamebrd.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t13
QTDIR_build:REQUIRES=full-config
unix:LIBS += -lm

