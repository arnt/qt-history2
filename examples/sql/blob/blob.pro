TEMPLATE	= app
TARGET          = blob

CONFIG		+= qt warn_on release
win32:CONFIG	+= console

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
