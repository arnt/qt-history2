TEMPLATE	= app
TARGET		= buttongroups

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, small-config)"

HEADERS		= buttongroups.h
SOURCES		= buttongroups.cpp \
		  main.cpp
