TEMPLATE	= app
TARGET		= picture

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= picture.cpp
