# Project ID used by some IDEs
GUID 		= {362ab0bf-4859-408f-a704-1e37cae18075}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t12
QTDIR_build:REQUIRES=full-config
unix:LIBS += -lm
