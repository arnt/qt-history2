TEMPLATE	= app
CONFIG		+= qt warn_on release
QT         += compat
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t12
QTDIR_build:REQUIRES=full-config
unix:LIBS += -lm
