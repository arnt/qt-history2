TEMPLATE	= app
TARGET		= iconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = iconview "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
