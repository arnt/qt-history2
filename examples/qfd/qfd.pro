TEMPLATE	= app
TARGET		= qfd

CONFIG		+= qt warn_on release
QT		+= compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= fontdisplayer.h
SOURCES		= fontdisplayer.cpp \
		  qfd.cpp
