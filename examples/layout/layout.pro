TEMPLATE	= app
TARGET		= layout

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		=
SOURCES		= layout.cpp
