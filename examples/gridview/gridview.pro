TEMPLATE	= app
TARGET		= gridview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, medium-config)"

HEADERS		=
SOURCES		= gridview.cpp

