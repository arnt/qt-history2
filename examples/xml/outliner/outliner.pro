TEMPLATE	= app
TARGET		= outliner

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= outlinetree.h
SOURCES		= main.cpp \
		  outlinetree.cpp
INTERFACES	=
QT	+= compat xml
