TEMPLATE	= app
TARGET		= statistics

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES 	= table "contains(QT_CONFIG, full-config)"

HEADERS		= statistics.h
SOURCES		= statistics.cpp main.cpp
