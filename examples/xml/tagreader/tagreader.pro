# Project ID used by some IDEs
GUID 		= {7340562a-3a9a-416c-88a4-304f31554938}
TEMPLATE	= app
TARGET          = tagreader

CONFIG		+= qt console warn_on release

QTDIR_build:REQUIRES	= xml large-config

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
