TEMPLATE	= app
TARGET		= dragdrop

CONFIG		+= qt warn_on release
QT              += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= dropsite.h \
		  secret.h
SOURCES		= dropsite.cpp \
		  main.cpp \
		  secret.cpp
