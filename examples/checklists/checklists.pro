TEMPLATE	= app
TARGET		= checklists

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= checklists.h
SOURCES		= checklists.cpp \
		  main.cpp
QT	+= compat
