GUID 		= {f840c44a-9993-4e01-a872-3c331b6becc9}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= lcdrange.h
SOURCES		= lcdrange.cpp \
		  main.cpp
TARGET		= t7
QTDIR_build:REQUIRES=large-config
