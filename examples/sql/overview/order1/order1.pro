TEMPLATE	= app
QCONFIG += sql
CONFIG		+= qt warn_on release
HEADERS		= 
SOURCES		= main.cpp ../connection.cpp
DEPENDPATH	=../../../include
QTDIR_build:REQUIRES=full-config
