TEMPLATE	= app
TARGET          = sqltable

CONFIG		+= qt warn_on release
QT		+= sql

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
