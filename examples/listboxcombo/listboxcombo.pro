TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= listboxcombo.h
SOURCES		= listboxcombo.cpp \
		  main.cpp
TARGET		= listboxcombo
QTDIR_build:REQUIRES=large-config
