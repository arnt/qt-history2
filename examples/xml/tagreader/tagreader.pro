TEMPLATE	= app
CONFIG		+= qt console warn_on release
HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
TARGET          = tagreader
REQUIRES	= xml large-config
