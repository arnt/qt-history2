QTDIR_build:REQUIRES        = embedded
TEMPLATE	= app
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include
HEADERS		= 
SOURCES		= launcher.cpp
TARGET		= launcher
QTDIR_build:REQUIRES=large-config
