TEMPLATE	= app
TARGET		= widgets

CONFIG		+= qt warn_on no_batch
QT         += compat
DEPENDPATH	= ../../include
INCLUDEPATH	+= ../aclock ../dclock

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= widgets.h ../aclock/aclock.h ../dclock/dclock.h
SOURCES		= main.cpp widgets.cpp ../aclock/aclock.cpp ../dclock/dclock.cpp
