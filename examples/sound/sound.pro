TEMPLATE	= app
TARGET		= sound

CONFIG		+= qt warn_on release
QT              += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"
x11:QTDIR_build:REQUIRES	= "contains(QT_CONFIG, nas)"

HEADERS		= sound.h
SOURCES		= sound.cpp
