TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannonfield.h \
		  lcdrange.h
SOURCES		= cannonfield.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t9
QTDIR_build:REQUIRES="contains(QT_CONFIG, full-config)"
