TEMPLATE	= app
CONFIG		+= qt warn_on
HEADERS		= cannonfield.h \
		  gameboard.h \
		  lcdrange.h
SOURCES		= cannonfield.cpp \
		  gameboard.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t14
QTDIR_build:REQUIRES="contains(QT_CONFIG, full-config)"
unix:LIBS += -lm
