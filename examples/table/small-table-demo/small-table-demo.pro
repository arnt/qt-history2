TEMPLATE	= app
TARGET		= smalltable

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES 	= table "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
