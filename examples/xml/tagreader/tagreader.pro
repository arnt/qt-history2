TEMPLATE	= app
TARGET          = tagreader

CONFIG		+= qt console warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
