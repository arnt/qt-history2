TEMPLATE	= app
TARGET		= drawlines

CONFIG		+= qt warn_on release
DEPENDPATH 	= ../../include

QTDIR_build:REQUIRES=

HEADERS		=
SOURCES		= connect.cpp
QT	+= compat
