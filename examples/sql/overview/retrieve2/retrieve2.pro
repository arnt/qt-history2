TEMPLATE	= app

QT		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

REQUIRES 	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp ../connection.cpp

