TEMPLATE	= app
TARGET		= listboxcombo

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= listboxcombo.h
SOURCES		= listboxcombo.cpp \
		  main.cpp
