GUID 		= {fd24a684-d589-46da-b141-e95574cd8ffa}
TEMPLATE	= app
TARGET		= menu

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= menu.h
SOURCES		= menu.cpp
