TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= lineedits.h
SOURCES		= lineedits.cpp \
		  main.cpp
TARGET		= lineedits
DEPENDPATH=../../include
QTDIR_build:REQUIRES=medium-config
