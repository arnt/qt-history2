GUID 		= {6515a4af-0f2c-411f-ac8c-cfff1f4741c5}
TEMPLATE	= app
TARGET		= prodcons

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= thread large-config

SOURCES		= prodcons.cpp
CLEAN_FILES	= prodcons.out
