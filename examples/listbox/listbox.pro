TEMPLATE	= app
TARGET		= listbox

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= listbox.h
SOURCES		= listbox.cpp \
		  main.cpp
