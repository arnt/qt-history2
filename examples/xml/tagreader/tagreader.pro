TEMPLATE	= app
TARGET          = tagreader

CONFIG		+= qt console warn_on release

QTDIR_build:REQUIRES	= xml large-config

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
