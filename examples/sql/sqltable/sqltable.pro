TEMPLATE	= app
QCONFIG += sql
CONFIG+= qt warn_on release
HEADERS		= 
SOURCES		= main.cpp
INTERFACES	= 
TARGET          = sqltable
QTDIR_build:REQUIRES=full-config
