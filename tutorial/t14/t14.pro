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
QTDIR_build:REQUIRES="contains(QT_CONFIG, full-config)"
unix:LIBS += -lm
