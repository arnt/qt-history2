TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= buttongroups.h
SOURCES		= buttongroups.cpp \
		  main.cpp
TARGET		= buttongroups
QTDIR_build:REQUIRES=small-config
