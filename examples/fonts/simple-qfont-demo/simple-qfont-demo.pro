TEMPLATE	= app
TARGET		= fontdemo

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES 	= "contains(QT_CONFIG, full-config)"

HEADERS		= viewer.h
SOURCES		= simple-qfont-demo.cpp \
	          viewer.cpp
QT	+= compat
