TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= lcdrange.h
SOURCES		= lcdrange.cpp \
		  main.cpp
TARGET		= t7
QTDIR_build:REQUIRES="contains(QT_CONFIG, large-config)"
