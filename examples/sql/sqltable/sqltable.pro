TEMPLATE	= app
TARGET          = sqltable

CONFIG		+= qt warn_on release
QCONFIG		+= sql

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
