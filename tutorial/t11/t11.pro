GUID 		= {c52e62ff-9437-4353-8ca3-0119b706d453}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t11
QTDIR_build:REQUIRES=full-config
unix:LIBS += -lm

