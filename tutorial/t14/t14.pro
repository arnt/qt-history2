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
REQUIRES=full-config
irix-n32:LIBS += -lm
