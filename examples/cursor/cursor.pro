TEMPLATE	= app
TARGET		= cursor

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, small-config)"

HEADERS		=
SOURCES		= cursor.cpp

