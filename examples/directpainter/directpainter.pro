TEMPLATE	= app

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = embedded "contains(QT_CONFIG, small-config)"

HEADERS		=
SOURCES		= directpainter.cpp
TARGET		= directpainter
